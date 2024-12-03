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

// LSU <> Data Memory Controller Channels
localparam int NUM_LSUS = NUM_CORES * THREADS_PER_WARP;
typedef logic [NUM_LSUS-1:0] lsu_size_t;
lsu_size_t lsu_read_valid;
lsu_size_t lsu_read_ready;
lsu_size_t lsu_write_valid;
lsu_size_t lsu_write_ready;
data_memory_address_t lsu_read_address [NUM_LSUS];
data_memory_address_t lsu_write_address [NUM_LSUS];
data_t lsu_read_data [NUM_LSUS];
data_t lsu_write_data [NUM_LSUS];

// Fetcher <> Program Memory Controller Channels
localparam NUM_FETCHERS = NUM_CORES;
typedef logic [NUM_FETCHERS-1:0] fetcher_size_t;
fetcher_size_t fetcher_read_valid;
instruction_memory_address_t fetcher_read_address [NUM_FETCHERS];
fetcher_size_t fetcher_read_ready;
instruction_t fetcher_read_data [NUM_FETCHERS];

dispatcher #(
    .NUM_CORES(NUM_CORES)
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

// Data Memory Controller
mem_controller #(
    .DATA_WIDTH(`DATA_WIDTH),
    .ADDRESS_WIDTH(`DATA_MEMORY_ADDRESS_WIDTH),
    .NUM_CONSUMERS(NUM_LSUS),
    .NUM_CHANNELS(DATA_MEM_NUM_CHANNELS)
    ) data_memory_controller (
        .clk(clk),
        .reset(reset),

        .consumer_read_valid(lsu_read_valid),
        .consumer_read_address(lsu_read_address),
        .consumer_read_ready(lsu_read_ready),
        .consumer_read_data(lsu_read_data),

        .consumer_write_valid(lsu_write_valid),
        .consumer_write_address(lsu_write_address),
        .consumer_write_data(lsu_write_data),
        .consumer_write_ready(lsu_write_ready),

        .mem_read_valid(data_mem_read_valid),
        .mem_read_address(data_mem_read_address),
        .mem_read_ready(data_mem_read_ready),
        .mem_read_data(data_mem_read_data),

        .mem_write_valid(data_mem_write_valid),
        .mem_write_address(data_mem_write_address),
        .mem_write_data(data_mem_write_data),
        .mem_write_ready(data_mem_write_ready)
    );

// Program Memory Controller
mem_controller #(
    .DATA_WIDTH(`INSTRUCTION_WIDTH),
    .ADDRESS_WIDTH(`INSTRUCTION_MEMORY_ADDRESS_WIDTH),
    .NUM_CONSUMERS(NUM_FETCHERS),
    .NUM_CHANNELS(INSTRUCTION_MEM_NUM_CHANNELS),
    .WRITE_ENABLE(0)
) program_memory_controller (
    .clk(clk),
    .reset(reset),

    .consumer_read_valid(fetcher_read_valid),
    .consumer_read_address(fetcher_read_address),
    .consumer_read_ready(fetcher_read_ready),
    .consumer_read_data(fetcher_read_data),

    .consumer_write_valid(0),
    .consumer_write_address(0),
    .consumer_write_data(0),
    .consumer_write_ready(0),

    .mem_read_valid(program_mem_read_valid),
    .mem_read_address(program_mem_read_address),
    .mem_read_ready(program_mem_read_ready),
    .mem_read_data(program_mem_read_data),

    .mem_write_valid(0),
    .mem_write_address(0),
    .mem_write_data(0),
    .mem_write_ready(0)
);


initial begin
    $display("Hello, World!");
end

endmodule
