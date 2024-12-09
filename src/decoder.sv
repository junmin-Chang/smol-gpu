`default_nettype none
`timescale 1ns/1ns

`include "common.sv"

module decoder(
    input instruction_t instruction,

    output logic decoded_reg_write_enable,
    output logic decoded_mem_write_enable,
    output logic decoded_mem_read_enable,
    output logic decoded_branch,
    output logic [1:0] decoded_reg_input_mux,
    output data_t decoded_immediate,
    output logic [4:0] decoded_rd_address,
    output logic [4:0] decoded_rs1_address,
    output logic [4:0] decoded_rs2_address,
    output alu_instruction_t decoded_alu_instruction,

    output logic decoded_finish
);
    // Extract fields from instruction
    wire [6:0] opcode = instruction[6:0];
    wire [4:0] rd     = instruction[11:7];
    wire [2:0] funct3 = instruction[14:12];
    wire [4:0] rs1    = instruction[19:15];
    wire [4:0] rs2    = instruction[24:20];
    wire [6:0] funct7 = instruction[31:25];
    wire [11:0] imm_i = instruction[31:20];
    wire [11:0] imm_s = {instruction[31:25], instruction[11:7]};
    wire [12:0] imm_b = {instruction[31], instruction[7], instruction[30:25], instruction[11:8], 1'b0};
    wire [31:12] imm_u = instruction[31:12];

    always_comb begin
        // Default assignments
        decoded_reg_write_enable = 0;
        decoded_reg_input_mux = 2'b00;
        decoded_immediate = {`DATA_WIDTH{1'b0}};
        decoded_rd_address = 5'b0;
        decoded_rs1_address = 5'b0;
        decoded_rs2_address = 5'b0;
        decoded_alu_instruction = ADDI;
        decoded_mem_read_enable = 0;
        decoded_mem_write_enable = 0;
        decoded_branch = 0;

        // Start decoding
        unique case (opcode)
            `OPCODE_R: begin
                // Vector R-type instructions
                decoded_rd_address = rd;
                decoded_rs1_address = rs1;
                decoded_rs2_address = rs2;
                decoded_reg_write_enable = 1;
                decoded_reg_input_mux = 2'b00; // ALU result
                decoded_mem_read_enable = 0;
                decoded_mem_write_enable = 0;
                decoded_branch = 0;

                // Determine the ALU instruction
                unique case (funct3)
                    3'b000: begin
                        if (funct7 == 7'b0000000)
                            decoded_alu_instruction = ADD;
                        else if (funct7 == 7'b0100000)
                            decoded_alu_instruction = SUB;
                        else
                            $error("Invalid R-type instruction with funct7 %b", funct7);
                    end
                    3'b001: decoded_alu_instruction = SLL;
                    3'b010: decoded_alu_instruction = SLT;
                    3'b100: decoded_alu_instruction = XOR;
                    3'b101: begin
                        if (funct7 == 7'b0000000)
                            decoded_alu_instruction = SRL;
                        else if (funct7 == 7'b0100000)
                            decoded_alu_instruction = SRA;
                        else
                            $error("Invalid R-type instruction with funct7 %b", funct7);
                    end
                    3'b110: decoded_alu_instruction = OR;
                    3'b111: decoded_alu_instruction = AND;
                    default: $error("Invalid R-type instruction with funct3 %b", funct3);
                endcase
            end
            `OPCODE_I: begin
                // Vector I-type instructions
                decoded_rd_address = rd;
                decoded_rs1_address = rs1;
                decoded_reg_write_enable = 1;
                decoded_reg_input_mux = 2'b00; // ALU result
                decoded_immediate = sign_extend(imm_i);
                decoded_mem_read_enable = 0;
                decoded_mem_write_enable = 0;
                decoded_branch = 0;

                unique case (funct3)
                    3'b000: decoded_alu_instruction = ADDI;
                    3'b010: decoded_alu_instruction = SLTI;
                    3'b100: decoded_alu_instruction = XORI;
                    3'b110: decoded_alu_instruction = ORI;
                    3'b111: decoded_alu_instruction = ANDI;
                    3'b001: decoded_alu_instruction = SLLI;
                    3'b101: begin
                        if (funct7 == 7'b0000000)
                            decoded_alu_instruction = SRLI;
                        else if (funct7 == 7'b0100000)
                            decoded_alu_instruction = SRAI;
                        else
                            $error("Invalid I-type shift instruction with funct7 %b", funct7);
                    end
                    default: $error("Invalid I-type instruction with funct3 %b", funct3);
                endcase
            end
            `OPCODE_I_LOAD: begin
                // Load instructions (e.g., LW)
                decoded_rd_address = rd;
                decoded_rs1_address = rs1;
                decoded_reg_write_enable = 1;
                decoded_reg_input_mux = 2'b01; // LSU result
                decoded_immediate = sign_extend(imm_i);
                decoded_mem_read_enable = 1;
                decoded_mem_write_enable = 0;
                decoded_alu_instruction = ADDI; // For computing effective address
                decoded_branch = 0;
            end
            `OPCODE_S: begin
                // Store instructions (e.g., SW)
                decoded_rs1_address = rs1;
                decoded_rs2_address = rs2;
                decoded_reg_write_enable = 0;
                decoded_reg_input_mux = 2'b00;
                decoded_immediate = sign_extend(imm_s);
                decoded_mem_read_enable = 0;
                decoded_mem_write_enable = 1;
                decoded_alu_instruction = ADDI; // For computing effective address
                decoded_branch = 0;
            end
            `OPCODE_B: begin
                // Branch instructions (e.g., BEQ, BNE)
                decoded_rs1_address = rs1;
                decoded_rs2_address = rs2;
                decoded_immediate = sign_extend_13(imm_b);
                decoded_reg_write_enable = 0;
                decoded_reg_input_mux = 2'b00;
                decoded_mem_read_enable = 0;
                decoded_mem_write_enable = 0;
                decoded_branch = 1;

                unique case (funct3)
                    3'b000: decoded_alu_instruction = BEQ;
                    3'b001: decoded_alu_instruction = BNE;
                    3'b100: decoded_alu_instruction = BLT;
                    3'b101: decoded_alu_instruction = BGE;
                    default: $error("Invalid B-type instruction with funct3 %b", funct3);
                endcase
            end
            `OPCODE_U: begin
                // LUI instruction
                decoded_rd_address = rd;
                decoded_immediate = {imm_u, 12'b0}; // Immediate value shifted left 12 bits
                decoded_reg_write_enable = 1;
                decoded_reg_input_mux = 2'b10; // Immediate
                decoded_mem_read_enable = 0;
                decoded_mem_write_enable = 0;
                decoded_branch = 0;
            end
            7'b1110011: begin
                // Scalar and vector-scalar instructions
                // Implement decoding based on your specific encoding
                // For example, you might have:
                // - If funct3 == 3'b010 and funct7 == 7'b0000000, it's SX_SLT
                // - If funct3 == 3'b011 and funct7 == 7'b0000000, it's SX_SLTI
                // Add your decoding logic here

                // Placeholder error for unimplemented decoding
                $error("Scalar and vector-scalar instruction decoding not implemented");
            end
            default: $warning("Unrecognized opcode %b", opcode);
        endcase
    end

endmodule
