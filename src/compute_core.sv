`default_nettype none
`timescale 1ns/1ns

`include "common.sv"

module compute_core#(
    parameter int WARPS_PER_CORE = 2,            // Number of warps to in each core
    parameter int THREADS_PER_WARP = 32          // Number of threads per warp (max 32)
    )(
    input wire clk,
    input wire reset,

    input logic start,
    output logic done,

    input data_t block_id,
    input kernel_config_t kernel_config,

    // Instruction Memory
    input logic [WARPS_PER_CORE-1:0] instruction_mem_read_ready,
    input instruction_t instruction_mem_read_data [WARPS_PER_CORE],
    output logic [WARPS_PER_CORE-1:0] instruction_mem_read_valid,
    output instruction_memory_address_t instruction_mem_read_address [WARPS_PER_CORE],

    // Data Memory
    output logic [NUM_LSUS-1:0] data_mem_read_valid,
    output data_memory_address_t data_mem_read_address [NUM_LSUS],
    input logic [NUM_LSUS-1:0] data_mem_read_ready,
    input data_t data_mem_read_data [NUM_LSUS],
    output logic [NUM_LSUS-1:0] data_mem_write_valid,
    output data_memory_address_t data_mem_write_address [NUM_LSUS],
    output data_t data_mem_write_data [NUM_LSUS],
    input logic [NUM_LSUS-1:0] data_mem_write_ready
);

typedef logic [THREADS_PER_WARP-1:0] warp_mask_t;
localparam int NUM_LSUS = THREADS_PER_WARP + 1;

// Warp specific variables
int current_warp;

warp_state_t warp_state [WARPS_PER_CORE];
fetcher_state_t fetcher_state_in [WARPS_PER_CORE];

warp_state_t current_warp_state;
assign current_warp_state = warp_state[current_warp];

data_t rs1 [WARPS_PER_CORE][THREADS_PER_WARP];
data_t rs2 [WARPS_PER_CORE][THREADS_PER_WARP];

instruction_t fetched_instruction [WARPS_PER_CORE];

// Alu specific variables
data_t alu_out [THREADS_PER_WARP];

// LSU specific variables
logic decoded_mem_read_enable [THREADS_PER_WARP];
logic decoded_mem_write_enable [THREADS_PER_WARP];
lsu_state_t lsu_state [THREADS_PER_WARP];
data_t lsu_out [THREADS_PER_WARP];

// Decoded instruction fields per warp
logic decoded_reg_write_enable [WARPS_PER_CORE];
reg_input_mux_t decoded_reg_input_mux [WARPS_PER_CORE];
data_t decoded_immediate [WARPS_PER_CORE];
logic decoded_branch [WARPS_PER_CORE];
logic decoded_scalar_instruction [WARPS_PER_CORE];
logic [4:0] decoded_rd_address [WARPS_PER_CORE];
logic [4:0] decoded_rs1_address [WARPS_PER_CORE];
logic [4:0] decoded_rs2_address [WARPS_PER_CORE];
logic [4:0] decoded_alu_instruction [WARPS_PER_CORE];
logic decoded_halt [WARPS_PER_CORE];

// scalar registers
warp_mask_t warp_execution_mask [WARPS_PER_CORE];
warp_mask_t current_warp_execution_mask;
assign current_warp_execution_mask = warp_execution_mask[current_warp];
data_t scalar_rs1 [WARPS_PER_CORE];
data_t scalar_rs2 [WARPS_PER_CORE];
data_t scalar_lsu_out;
data_t scalar_alu_out;
lsu_state_t scalar_lsu_state;

data_t vector_to_scalar_data [WARPS_PER_CORE];

instruction_memory_address_t pc [WARPS_PER_CORE];
instruction_memory_address_t next_pc [WARPS_PER_CORE];

logic start_execution; // EDA: Unimportant hack used because of EDA tooling

data_t num_warps;
assign num_warps = kernel_config.num_warps_per_block;

alu warp_alu_inst(
    .clk(clk),
    .reset(reset),
    .enable(decoded_scalar_instruction[current_warp]),

    .pc(pc[current_warp]),
    .rs1(scalar_rs1[current_warp]),
    .rs2(scalar_rs2[current_warp]),
    .imm(decoded_immediate[current_warp]),
    .instruction(alu_instruction_t'(decoded_alu_instruction[current_warp])),

    .alu_out(scalar_alu_out)
);

lsu warp_lsu_inst (
    .clk(clk),
    .reset(reset),
    .enable(decoded_scalar_instruction[current_warp]),

    .warp_state(warp_state[current_warp]),

    .decoded_mem_read_enable(decoded_mem_read_enable[current_warp]),
    .decoded_mem_write_enable(decoded_mem_write_enable[current_warp]),

    .rs1(scalar_rs1[current_warp]),
    .rs2(scalar_rs2[current_warp]),
    .imm(decoded_immediate[current_warp]),

    // Data Memory connections
    .mem_read_valid(data_mem_read_valid[THREADS_PER_WARP]),
    .mem_read_address(data_mem_read_address[THREADS_PER_WARP]),
    .mem_read_ready(data_mem_read_ready[THREADS_PER_WARP]),
    .mem_read_data(data_mem_read_data[THREADS_PER_WARP]),
    .mem_write_valid(data_mem_write_valid[THREADS_PER_WARP]),
    .mem_write_address(data_mem_write_address[THREADS_PER_WARP]),
    .mem_write_data(data_mem_write_data[THREADS_PER_WARP]),
    .mem_write_ready(data_mem_write_ready[THREADS_PER_WARP]),

    .lsu_state(scalar_lsu_state),
    .lsu_out(scalar_lsu_out)
);

always @(posedge clk) begin
	int next_warp;
	int found_warp;
	int warp_index;
	if (reset) begin
        $display("Resetting core %0d", block_id);
        start_execution <= 0;
        done <= 0;
        for (int i = 0; i < WARPS_PER_CORE; i = i + 1) begin
            warp_state[i] <= WARP_IDLE;
            fetcher_state_in[i] <= FETCHER_IDLE;
            pc[i] <= 0;
            next_pc[i] <= 0;
            current_warp <= 0;
        end
    end else if (!start_execution) begin
        if (start) begin
            $display("Starting execution of block %d", block_id);
            // Set all warps to fetch state on start
            start_execution <= 1;
            current_warp <= 0;
            for (int i = 0; i < WARPS_PER_CORE; i = i + 1) begin
                warp_state[i] <= WARP_FETCH;
                fetcher_state_in[i] <= FETCHER_IDLE;
                pc[i] <= kernel_config.base_instructions_address;
                next_pc[i] <= kernel_config.base_instructions_address;
            end
        end
    end else begin
        // In parallel, check if fetchers are done, and if so, move to decode
        for (int i = 0; i < WARPS_PER_CORE; i = i + 1) begin
            if (warp_state[i] == WARP_FETCH && fetcher_state_in[i] == FETCHER_DONE) begin
                $display("Block: %0d: Warp %0d: Fetched instruction %h at address %h", block_id, i, fetched_instruction[i], pc[i]);
                warp_state[i] <= WARP_DECODE;
            end
        end

        // If all warps are done, we are done
        for (int i = 0; i < WARPS_PER_CORE; i = i + 1) begin
            if (warp_state[i] != WARP_DONE) begin
                done <= 0;
                break;
            end
            done <= 1;
        end

        // Choose a warp to execute
        // We don't choose warps that are in one of the following states:
        // - WARP_IDLE - that means that the warp is not active
        // - WARP_DONE - that means that the warp has finished execution
        // - WARP_FETCH - that means that the warp is fetching instructions
        // For now we do not change state unless we are in WARP_UPDATE

        if (current_warp_state == WARP_UPDATE || current_warp_state == WARP_DONE) begin
            next_warp = (current_warp + 1) % WARPS_PER_CORE;
            found_warp = -1;
            $display("Block: %0d: Choosing next warp", block_id);
            for (int i = 0; i < WARPS_PER_CORE; i = i + 1) begin
                warp_index = (next_warp + i) % WARPS_PER_CORE;
                if ((warp_state[warp_index] != WARP_IDLE) && (warp_state[warp_index] != WARP_FETCH) && (warp_state[warp_index] != WARP_DONE)) begin
                    found_warp = warp_index;
                    break;
                end
            end
            if (found_warp != -1) begin
                current_warp <= found_warp;
            end else begin
                // No active warp ready; remain with current warp
                current_warp <= current_warp;
            end
        end

        case (current_warp_state)
            WARP_IDLE: begin
                $display("Block: %0d: Warp %0d: Idle", block_id, current_warp);
            end
            WARP_FETCH: begin
                // not possible to choose a warp that is fetching cause
                // fetching is done in parallel
            end
            WARP_DECODE: begin
                // decoding takes one cycle
                warp_state[current_warp] <= WARP_REQUEST;
            end
            WARP_REQUEST: begin
                // takes one cycle cause we are just changing the LSU state
                warp_state[current_warp] <= WARP_WAIT;
            end
            WARP_WAIT: begin
                reg any_lsu_waiting = 1'b0;
                for (int i = 0; i < THREADS_PER_WARP; i++) begin
                    // Make sure no lsu_state = REQUESTING or WAITING
                    if (lsu_state[i] == LSU_REQUESTING || lsu_state[i] == LSU_WAITING) begin
                        any_lsu_waiting = 1'b1;
                        break;
                    end
                end

                if (scalar_lsu_state == LSU_REQUESTING || scalar_lsu_state == LSU_WAITING) begin
                    any_lsu_waiting = 1'b1;
                end

                // If no LSU is waiting for a response, move onto the next stage
                if (!any_lsu_waiting) begin
                    warp_state[current_warp] <= WARP_EXECUTE;
                end
            end
            WARP_EXECUTE: begin
                $display("===================================");
                $display("Mask: %32b", warp_execution_mask[current_warp]);
                $display("Block: %0d: Warp %0d: Executing instruction %h at address %h", block_id, current_warp, fetched_instruction[current_warp], pc[current_warp]);
                $display("Instruction opcode: %b", fetched_instruction[current_warp][6:0]);
                if (decoded_scalar_instruction[current_warp]) begin
                    if (decoded_branch[current_warp]) begin
                        // Branch instruction
                        if (scalar_alu_out == 1) begin
                            // Branch taken
                            next_pc[current_warp] <= pc[current_warp] + decoded_immediate[current_warp];
                        end else begin
                            // Branch not taken
                            next_pc[current_warp] <= pc[current_warp] + 1;
                        end
                    end else if (decoded_alu_instruction[current_warp] == JAL) begin
                        next_pc[current_warp] <= scalar_alu_out;
                    end else if (decoded_alu_instruction[current_warp] == JALR) begin
                        next_pc[current_warp] <= scalar_alu_out;
                    end else begin
                        // Other scalar instruction
                        next_pc[current_warp] <= pc[current_warp] + 1;
                    end
                end else begin
                    // Vector instruction
                    next_pc[current_warp] <= pc[current_warp] + 1;
                end
                $display("===================================");
                warp_state[current_warp] <= WARP_UPDATE;

                if (decoded_reg_input_mux[current_warp] == VECTOR_TO_SCALAR) begin
                    data_t scalar_write_value;
                    scalar_write_value = {`DATA_WIDTH{1'b0}};
                    for (int i = 0; i < THREADS_PER_WARP; i++) begin
                        scalar_write_value[i] = alu_out[i][0];
                    end
                    vector_to_scalar_data[current_warp] <= scalar_write_value;
                end else begin
                    vector_to_scalar_data[current_warp] <= {`DATA_WIDTH{1'b0}};
                end

            end
            WARP_UPDATE: begin
                if (decoded_halt[current_warp]) begin
                    $display("Block: %0d: Warp %0d: Finished executing instruction %h", block_id, current_warp, fetched_instruction[current_warp]);
                    warp_state[current_warp] <= WARP_DONE;
                end else begin
                    pc[current_warp] <= next_pc[current_warp];
                    warp_state[current_warp] <= WARP_FETCH;
                end
            end
            WARP_DONE: begin
                // we chillin
            end
        endcase
    end
end

// This block generates warp control circuitry
generate
genvar warp_i;
for (warp_i = 0; warp_i < WARPS_PER_CORE; warp_i = warp_i + 1) begin : g_warp
    fetcher fetcher_inst(
        .clk(clk),
        .reset(reset),

        .warp_state(warp_state[warp_i]),
        .pc(pc[warp_i]),

        // Instruction Memory
        .instruction_mem_read_ready(instruction_mem_read_ready[warp_i]),
        .instruction_mem_read_data(instruction_mem_read_data[warp_i]),
        .instruction_mem_read_valid(instruction_mem_read_valid[warp_i]),
        .instruction_mem_read_address(instruction_mem_read_address[warp_i]),

        // Fetcher output
        .fetcher_state_in(fetcher_state_in[warp_i]),
        .instruction(fetched_instruction[warp_i])
    );

    decoder decoder_inst(
        .clk(clk),
        .reset(reset),
        .warp_state(warp_state[warp_i]),

        .instruction(fetched_instruction[warp_i]),

        .decoded_reg_write_enable(decoded_reg_write_enable[warp_i]),
        .decoded_mem_write_enable(decoded_mem_write_enable[warp_i]),
        .decoded_mem_read_enable(decoded_mem_read_enable[warp_i]),
        .decoded_branch(decoded_branch[warp_i]),
        .decoded_scalar_instruction(decoded_scalar_instruction[warp_i]),
        .decoded_reg_input_mux(decoded_reg_input_mux[warp_i]),
        .decoded_immediate(decoded_immediate[warp_i]),
        .decoded_rd_address(decoded_rd_address[warp_i]),
        .decoded_rs1_address(decoded_rs1_address[warp_i]),
        .decoded_rs2_address(decoded_rs2_address[warp_i]),
        .decoded_alu_instruction(decoded_alu_instruction[warp_i]),

        .decoded_halt(decoded_halt[warp_i])
    );

    scalar_reg_file #(
        .DATA_WIDTH(32)
    ) scalar_reg_file_inst (
        .clk(clk),
        .reset(reset),
        .enable((current_warp == warp_i)), // Enable when current_warp matches and warp is active

        .warp_execution_mask(warp_execution_mask[warp_i]),

        .warp_state(warp_state[warp_i]),

        .decoded_reg_write_enable(decoded_reg_write_enable[warp_i] & (decoded_scalar_instruction[warp_i] | decoded_reg_input_mux[current_warp] == VECTOR_TO_SCALAR)),
        .decoded_reg_input_mux(decoded_reg_input_mux[warp_i]),
        .decoded_immediate(decoded_immediate[warp_i]),
        .decoded_rd_address(decoded_rd_address[warp_i]),
        .decoded_rs1_address(decoded_rs1_address[warp_i]),
        .decoded_rs2_address(decoded_rs2_address[warp_i]),

        .alu_out(scalar_alu_out),
        .lsu_out(scalar_lsu_out),
        .pc(pc[warp_i]),
        .vector_to_scalar_data(vector_to_scalar_data[warp_i]),

        .rs1(scalar_rs1[warp_i]),
        .rs2(scalar_rs2[warp_i])
    );

    reg_file #(
            .THREADS_PER_WARP(THREADS_PER_WARP)
        ) reg_file_inst (
            .clk(clk),
            .reset(reset),
            .enable((current_warp == warp_i)), // Enable when current_warp matches and warp is active

            // Thread enable signals (execution mask)
            .thread_enable(warp_execution_mask[warp_i]),

            // Warp and block identifiers
            .warp_id(warp_i),
            .block_id(block_id),
            .block_size(kernel_config.num_warps_per_block * THREADS_PER_WARP),
            .warp_state(warp_state[warp_i]),

            // Decoded instruction fields for this warp
            .decoded_reg_write_enable(decoded_reg_write_enable[warp_i] & !decoded_scalar_instruction[warp_i]),
            .decoded_reg_input_mux(decoded_reg_input_mux[warp_i]),
            .decoded_immediate(decoded_immediate[warp_i]),
            .decoded_rd_address(decoded_rd_address[warp_i]),
            .decoded_rs1_address(decoded_rs1_address[warp_i]),
            .decoded_rs2_address(decoded_rs2_address[warp_i]),

            // Inputs from ALU and LSU per thread
            .alu_out(alu_out), // ALU outputs for all threads
            .lsu_out(lsu_out),

            // Outputs per thread
            .rs1(rs1[warp_i]),
            .rs2(rs2[warp_i])
        );
end
endgenerate


// This block generates shared core resources
generate
	genvar thread_i;
    for (thread_i = 0; thread_i < THREADS_PER_WARP; thread_i = thread_i + 1) begin : g_alus
        wire t_enable = current_warp_execution_mask[thread_i] & !decoded_scalar_instruction[current_warp];
        alu alu_inst(
            .clk(clk),
            .reset(reset),
            .enable(t_enable),

            .pc(pc[current_warp]),
            .rs1(rs1[current_warp][thread_i]),
            .rs2(rs2[current_warp][thread_i]),
            .imm(decoded_immediate[current_warp]),
            .instruction(alu_instruction_t'(decoded_alu_instruction[current_warp])),

            .alu_out(alu_out[thread_i])
        );

        lsu lsu_inst(
            .clk(clk),
            .reset(reset),
            .enable(t_enable),

            .warp_state(warp_state[current_warp]),

            .decoded_mem_read_enable(decoded_mem_read_enable[current_warp]),
            .decoded_mem_write_enable(decoded_mem_write_enable[current_warp]),

            .rs1(rs1[current_warp][thread_i]),
            .rs2(rs2[current_warp][thread_i]),
            .imm(decoded_immediate[current_warp]),

            // Data Memory connections
            .mem_read_valid(data_mem_read_valid[thread_i]),
            .mem_read_address(data_mem_read_address[thread_i]),
            .mem_read_ready(data_mem_read_ready[thread_i]),
            .mem_read_data(data_mem_read_data[thread_i]),
            .mem_write_valid(data_mem_write_valid[thread_i]),
            .mem_write_address(data_mem_write_address[thread_i]),
            .mem_write_data(data_mem_write_data[thread_i]),
            .mem_write_ready(data_mem_write_ready[thread_i]),

            .lsu_state(lsu_state[thread_i]),
            .lsu_out(lsu_out[thread_i])
        );
    end
endgenerate
endmodule
