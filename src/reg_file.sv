`default_nettype none
`timescale 1ns/1ns

`include "common.sv"

module reg_file (
    input wire clk,
    input wire reset,
    input wire enable,

    input data_t thread_id,
    input data_t block_id,
    input data_t block_size,

    input logic [1:0] decoded_reg_input_mux,

    input logic decoded_reg_write_enable,
    input logic [1:0] decoded_reg_input_mux,
    input data_t decoded_immediate,

    input logic [4:0] decoded_rd_address,
    input logic [4:0] decoded_rs1_address,
    input logic [4:0] decoded_rs2_address,

    input data_t alu_out,
    input data_t lsu_out,

    output data_t rd,
    output data_t rs1,
    output data_t rs2
);

// x0 is hardwired to 0
// x1 is hardwired to thread_id
// x2 is hardwired to block_id
// x3 is hardwired to block_size

localparam ARITHMETIC = 2'b00,
        MEMORY = 2'b01,
        CONSTANT = 2'b10;

data_t registers [32];

always @(posedge clk) begin
    if (reset) begin
        registers[0] <= 0;
        registers[1] <= thread_id;
        registers[2] <= block_id;
        registers[3] <= block_size;
        for (int i = 4; i < 32; i++) begin
            registers[i] <= 0;
        end
        rd <= 0;
        rs1 <= 0;
        rs2 <= 0;
    end else if (enable) begin
        if (decoded_reg_write_enable) begin
            case (decoded_reg_input_mux)
                ARITHMETIC: begin
                    // ADD, SUB, MUL, DIV
                    registers[decoded_rd_address] <= alu_out;
                end
                MEMORY: begin
                    // LDR
                    registers[decoded_rd_address] <= lsu_out;
                end
                CONSTANT: begin
                    // CONST
                    registers[decoded_rd_address] <= decoded_immediate;
                end
                default: begin
                    $error("Invalid decoded_reg_input_mux value");
                end
            endcase
        end
    end
end

endmodule
