// common.sv
`ifndef COMMON_SV
`define COMMON_SV

// Global Macros
`define DATA_WIDTH 32
`define INSTRUCTION_WIDTH 32
`define DATA_MEMORY_ADDRESS_WIDTH 32
`define INSTRUCTION_MEMORY_ADDRESS_WIDTH 32

// Type Definitions
typedef logic [`DATA_WIDTH-1:0] data_t;
typedef logic [`INSTRUCTION_WIDTH-1:0] instruction_t;
typedef logic [`DATA_MEMORY_ADDRESS_WIDTH-1:0] data_memory_address_t;
typedef logic [`INSTRUCTION_MEMORY_ADDRESS_WIDTH-1:0] instruction_memory_address_t;

typedef logic [7:0] num_blocks_t;
typedef logic [7:0] num_warps_per_block_t;

typedef struct packed {
    instruction_memory_address_t base_instructions_address;
    data_memory_address_t base_data_address; // This is where kernel function arguments are stored
    num_blocks_t num_blocks;
    num_warps_per_block_t num_warps_per_block;
} kernel_config_t;

`endif // COMMON_SV
