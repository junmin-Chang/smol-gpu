`default_nettype none
`timescale 1ns/1ns

module compute_core#(
    parameter int WARPS_PER_CORE = 4,            // Number of warps to in each core
    parameter int THREADS_PER_WARP = 32          // Number of threads per warp (max 32)
    )(
    input wire clk,
    input wire reset,

    input logic start,
    output logic done,

    input logic [7:0] block_id,
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

// Warp specific variables
warp_state_t warp_state [WARPS_PER_CORE];
fetcher_state_t fetcher_state [WARPS_PER_CORE];
instruction_memory_address_t pc [WARPS_PER_CORE];
instruction_t fetched_instruction [WARPS_PER_CORE];

// Alu specific variables
alu_input_t alu_input [THREADS_PER_WARP];
data_t alu_out [THREADS_PER_WARP];

// warp scheduler
// core memory controller

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
end

endgenerate

// This block generates per thread units
generate
for (genvar i = 0; i < THREADS_PER_WARP; i = i + 1) begin : g_thread
    alu alu_inst(
        .clk(clk),
        .reset(reset),
        .enable(1'b1),

        .alu_input(alu_input[i]),

        .alu_out(alu_out[i])
    );
end

endgenerate


endmodule
