`default_nettype none
`timescale 1ns/1ns

`include "common.sv"

module compute_core#(
    parameter int WARPS_PER_CORE = 4,            // Number of warps to in each core
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
    output logic [THREADS_PER_WARP-1:0] data_mem_read_valid,
    output data_memory_address_t data_mem_read_address [THREADS_PER_WARP],
    input logic [THREADS_PER_WARP-1:0] data_mem_read_ready,
    input data_t data_mem_read_data [THREADS_PER_WARP],
    output logic [THREADS_PER_WARP-1:0] data_mem_write_valid,
    output data_memory_address_t data_mem_write_address [THREADS_PER_WARP],
    output data_t data_mem_write_data [THREADS_PER_WARP],
    input logic [THREADS_PER_WARP-1:0] data_mem_write_ready
);

typedef logic [THREADS_PER_WARP-1:0] warp_mask_t;

// Warp specific variables
int current_warp;

warp_state_t warp_state [WARPS_PER_CORE];
fetcher_state_t fetcher_state [WARPS_PER_CORE];

warp_state_t chosen_warp_state = warp_state[current_warp];

instruction_memory_address_t pc [WARPS_PER_CORE];
instruction_memory_address_t next_pc [WARPS_PER_CORE];
data_t rs1_array[THREADS_PER_WARP][WARPS_PER_CORE];
data_t rs2_array[THREADS_PER_WARP][WARPS_PER_CORE];
data_t rt_array[THREADS_PER_WARP][WARPS_PER_CORE];

data_t rs1 [THREADS_PER_WARP];
data_t rs2 [THREADS_PER_WARP];

instruction_t fetched_instruction [WARPS_PER_CORE];

warp_mask_t warp_execution_mask [WARPS_PER_CORE];
warp_mask_t current_warp_execution_mask;
assign current_warp_execution_mask = warp_execution_mask[current_warp];

// Alu specific variables
data_t alu_out [THREADS_PER_WARP];

// LSU specific variables
logic decoded_mem_read_enable [THREADS_PER_WARP];
logic decoded_mem_write_enable [THREADS_PER_WARP];
lsu_state_t lsu_state [THREADS_PER_WARP];
data_t lsu_out [THREADS_PER_WARP];

// Decoded instruction fields per warp
logic decoded_reg_write_enable [WARPS_PER_CORE];
logic [1:0] decoded_reg_input_mux [WARPS_PER_CORE];
data_t decoded_immediate [WARPS_PER_CORE];
logic [4:0] decoded_rd_address [WARPS_PER_CORE];
logic [4:0] decoded_rs1_address [WARPS_PER_CORE];
logic [4:0] decoded_rs2_address [WARPS_PER_CORE];
logic [4:0] decoded_alu_instruction [WARPS_PER_CORE];

// This block generates warp control circuitry
generate
for (genvar i = 0; i < WARPS_PER_CORE; i = i + 1) begin : g_warp
    fetcher fetcher_inst(
        .clk(clk),
        .reset(reset),

        .warp_state(warp_state[i]),
        .pc(pc[i]),

        // Instruction Memory
        .instruction_mem_read_ready(instruction_mem_read_ready[i]),
        .instruction_mem_read_data(instruction_mem_read_data[i]),
        .instruction_mem_read_valid(instruction_mem_read_valid[i]),
        .instruction_mem_read_address(instruction_mem_read_address[i]),

        // Fetcher output
        .fetcher_state(fetcher_state[i]),
        .instruction(fetched_instruction[i])
    );

    reg_file #(
            .THREADS_PER_WARP(THREADS_PER_WARP)
        ) reg_file_inst (
            .clk(clk),
            .reset(reset),
            .enable((current_warp == i)), // Enable when current_warp matches and warp is active

            // Thread enable signals (execution mask)
            .thread_enable(warp_execution_mask[i]),

            // Warp and block identifiers
            .warp_id(i),
            .block_id(block_id),
            .block_size(kernel_config.num_warps_per_block * THREADS_PER_WARP),

            // Decoded instruction fields for this warp
            .decoded_reg_write_enable(decoded_reg_write_enable[i]),
            .decoded_reg_input_mux(decoded_reg_input_mux[i]),
            .decoded_immediate(decoded_immediate[i]),
            .decoded_rd_address(decoded_rd_address[i]),
            .decoded_rs1_address(decoded_rs1_address[i]),
            .decoded_rs2_address(decoded_rs2_address[i]),

            // Inputs from ALU and LSU per thread
            .alu_out(alu_out), // ALU outputs for all threads
            .lsu_out(lsu_out),

            // Outputs per thread
            .rs1(rs1),
            .rs2(rs2)
        );
end
endgenerate


// This block generates shared core resources

// ALU inputs
generate
    for (genvar i = 0; i < THREADS_PER_WARP; i = i + 1) begin : g_alus
        alu alu_inst(
            .clk(clk),
            .reset(reset),
            .enable(current_warp_execution_mask[i]),

            .rs1(rs1[i]),
            .rs2(rs2[i]),
            .imm12(decoded_immediate[current_warp][11:0]),
            .instruction(decoded_alu_instruction[current_warp]),

            .alu_out(alu_out[i])
        );
    end
endgenerate

// LSU inputs
generate
    for (genvar i = 0; i < THREADS_PER_WARP; i = i + 1) begin : g_lsus
        lsu lsu_inst(
            .clk(clk),
            .reset(reset),
            .enable(current_warp_execution_mask[i]),

            .warp_state(warp_state[current_warp]),

            .decoded_mem_read_enable(decoded_mem_read_enable[current_warp]),
            .decoded_mem_write_enable(decoded_mem_write_enable[current_warp]),

            .rs(rs1[i]),
            .rt(rs2[i]),

            // Data Memory connections
            .mem_read_valid(data_mem_read_valid[i]),
            .mem_read_address(data_mem_read_address[i]),
            .mem_read_ready(data_mem_read_ready[i]),
            .mem_read_data(data_mem_read_data[i]),
            .mem_write_valid(data_mem_write_valid[i]),
            .mem_write_address(data_mem_write_address[i]),
            .mem_write_data(data_mem_write_data[i]),
            .mem_write_ready(data_mem_write_ready[i]),

            .lsu_state(lsu_state[i]),
            .lsu_out(lsu_out[i])
        );
    end
endgenerate
endmodule
