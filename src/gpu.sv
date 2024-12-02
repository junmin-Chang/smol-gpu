`default_nettype none
`timescale 1ns/1ns

`include "common.sv"

module gpu #(
    parameter int DATA_MEM_NUM_CHANNELS = 4,     // Number of concurrent channels for sending requests to data memory
    parameter int INSTRUCTION_MEM_NUM_CHANNELS = 4,     // Number of concurrent channels for sending requests to data memory
    parameter int NUM_CORES = 2,                 // Number of cores to include in this GPU
    parameter int WARPS_PER_CORE = 4,            // Number of warps to in each core
    parameter int THREADS_PER_WARP = 32          // Number of threads per warp (max 32)
) (
    input wire clk,
    input wire reset,

    input wire execution_start,
    output wire execution_done,

    // kernel configuration
    input kernel_config_t kernel_config,

    // Program Memory
    output wire [INSTRUCTION_MEM_NUM_CHANNELS-1:0] instruction_mem_read_valid,
    output instruction_memory_address_t instruction_mem_read_address [INSTRUCTION_MEM_NUM_CHANNELS],
    input wire [INSTRUCTION_MEM_NUM_CHANNELS-1:0] instruction_mem_read_ready,
    input instruction_t instruction_mem_read_data [INSTRUCTION_MEM_NUM_CHANNELS],

    // Data Memory
    output wire [DATA_MEM_NUM_CHANNELS-1:0] data_mem_read_valid,
    output data_memory_address_t data_mem_read_address [DATA_MEM_NUM_CHANNELS],
    input wire [DATA_MEM_NUM_CHANNELS-1:0] data_mem_read_ready,
    input data_memory_address_t data_mem_read_data [DATA_MEM_NUM_CHANNELS],
    output wire [DATA_MEM_NUM_CHANNELS-1:0] data_mem_write_valid,
    output data_memory_address_t data_mem_write_address [DATA_MEM_NUM_CHANNELS],
    output data_t data_mem_write_data [DATA_MEM_NUM_CHANNELS],
    input wire [DATA_MEM_NUM_CHANNELS-1:0] data_mem_write_ready
);

logic [NUM_CORES-1:0] core_done;
logic [NUM_CORES-1:0] core_start;
logic [NUM_CORES-1:0] core_reset;
logic [7:0] core_block_id [NUM_CORES];

dispatcher #(
    .NUM_CORES(NUM_CORES),
    .WARPS_PER_CORE(WARPS_PER_CORE),
    .THREADS_PER_WARP(THREADS_PER_WARP)
    ) dispatcher_inst (
    .clk(clk),
    .reset(reset),
    .start(execution_start),

    .kernel_config_reg(kernel_config),

    .core_done(core_done),
    .core_start(core_start),
    .core_reset(core_reset),
    .core_block_id(core_block_id),

    .done(execution_done)
);

initial begin
    $display("Hello, World!");
end

endmodule
