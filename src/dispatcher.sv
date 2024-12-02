`default_nettype none
`timescale 1ns/1ns

`include "common.sv"

module dispatcher #(
    parameter int NUM_CORES = 2,                 // Number of cores to include in this GPU
) (
    input wire clk,
    input wire reset,
    input wire start,

    // Kernel Metadata
    input kernel_config_t kernel_config_reg,

    // Core States
    input reg [NUM_CORES-1:0] core_done,
    output reg [NUM_CORES-1:0] core_start,
    output reg [NUM_CORES-1:0] core_reset,
    output reg [7:0] core_block_id [NUM_CORES],

    // Kernel Execution
    output reg done
);

endmodule
