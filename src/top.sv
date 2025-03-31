module top (
    input  logic clk,
    input  logic rst_n,
    input  logic btn_start,      
    output logic [7:0] leds       
);

    logic rst;
    assign rst = ~rst_n;

    logic execution_start;
    logic execution_done;

    assign execution_start = btn_start;

    kernel_config_t kernel_config = '{
        base_instructions_address: instruction_memory_address_t'(32'h00000000),
        base_data_address:         data_memory_address_t'(32'h00000000),
        num_blocks:                data_t'(32'd1),
        num_warps_per_block:       data_t'(32'd2)
    };

    logic [7:0] instruction_mem_read_valid;
    instruction_memory_address_t instruction_mem_read_address [8];
    logic [7:0] instruction_mem_read_ready;
    instruction_t instruction_mem_read_data [8];

    logic [7:0] data_mem_read_valid;
    data_memory_address_t data_mem_read_address [8];
    logic [7:0] data_mem_read_ready;
    data_memory_address_t data_mem_read_data [8];
    logic [7:0] data_mem_write_valid;
    data_memory_address_t data_mem_write_address [8];
    data_t data_mem_write_data [8];
    logic [7:0] data_mem_write_ready;

    gpu #(
        .DATA_MEM_NUM_CHANNELS(8),
        .INSTRUCTION_MEM_NUM_CHANNELS(8),
        .NUM_CORES(1),
        .WARPS_PER_CORE(2),
        .THREADS_PER_WARP(32)
    ) gpu_inst (
        .clk(clk),
        .reset(rst),
        .execution_start(execution_start),
        .execution_done(execution_done),
        .kernel_config(kernel_config),
        .instruction_mem_read_valid(instruction_mem_read_valid),
        .instruction_mem_read_address(instruction_mem_read_address),
        .instruction_mem_read_ready(instruction_mem_read_ready),
        .instruction_mem_read_data(instruction_mem_read_data),
        .data_mem_read_valid(data_mem_read_valid),
        .data_mem_read_address(data_mem_read_address),
        .data_mem_read_ready(data_mem_read_ready),
        .data_mem_read_data(data_mem_read_data),
        .data_mem_write_valid(data_mem_write_valid),
        .data_mem_write_address(data_mem_write_address),
        .data_mem_write_data(data_mem_write_data),
        .data_mem_write_ready(data_mem_write_ready)
    );

    // 실행 완료 신호를 LED로 표시
    assign leds = {7'b0, execution_done};

endmodule