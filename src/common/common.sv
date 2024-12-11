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

typedef struct packed {
    instruction_memory_address_t base_instructions_address;
    data_memory_address_t base_data_address; // This is where kernel function arguments are stored
    data_t num_blocks;
    data_t num_warps_per_block;
} kernel_config_t;

// RISC-V Definitions
`define OPCODE_WIDTH 7
`define FUNCT3_WIDTH 3
`define FUNCT7_WIDTH 7

// Finish Instruction Opcode
`define OPCODE_FINISH 7'b1111111

// Vector Instruction Opcodes
`define OPCODE_R 7'b0110011         // Used by all R-type instructions (ADD, SUB, SLL, SLT, XOR, SRL, SRA)
`define OPCODE_I 7'b0010011         // Used by ALU I-type instructions (ADDI, SLTI, XORI, ORI, ANDI, SLLI, SRLI, SRAI)
`define OPCODE_S 7'b0100011         // Used by store instructions (SB, SH, SW)
`define OPCODE_B 7'b1100011         // Used by branch instructions (BEQ, BNE, BLT, BGE)
`define OPCODE_U 7'b0110111         // Used by LUI
// `define OPCODE_J 7'b1101111      // Used by JAL but it is not a vector instruction, jumps are scalar
`define OPCODE_I_LOAD 7'b0000011    // Used by load instructions (LB, LH, LW)
`define OPCODE_AUIPC 7'b0010111     // Used by AUIPC

// Scalar Instruction Opcodes
`define OPCODE_SCALAR_R 7`b1110011  // Used by all scalar R-type instructions (ADD, SUB, SLL, SLT, XOR, SRL, SRA, OR, AND)
`define OPCODE_SCALAR_I 7`b1110011  // Used by all scalar I-type instructions (ADDI, SLTI, XORI, ORI, ANDI, SLLI, SRLI, SRAI)
`define OPCODE_SCALAR_S 7`b1110011  // Used by all scalar S-type instructions (SB, SH, SW)
`define OPCODE_SCALAR_B 7`b1110011  // Used by all scalar B-type instructions (BEQ, BNE, BLT, BGE)
`define OPCODE_SCALAR_U 7`b1110011  // Used by all scalar U-type instructions (LUI)
`define OPCODE_SCALAR_J 7`b1110011  // Used by all scalar J-type instructions (JAL)
`define OPCODE_SCALAR_I_LOAD 7`b1110011 // Used by all scalar I-type instructions (LB, LH, LW)
`define OPCODE_SCALAR_AUIPC 7`b1110011  // Used by all scalar AUIPC instructions

// Vector-Scalar Instruction Opcodes (SX_SLTI and SX_SLT)
// SX_SLTI sets one bit of a scalar register based on thread's comparison result
// SX_SLT rd, rs1, rs2 <=> rd[id] = rs1 < rs2 ? 1 : 0
// SX_SLTI rd, rs1, imm <=> rd[id] = rs1 < imm ? 1 : 0
`define OPCODE_VECTOR_SCALAR_R 7`b1110011  // Used by all vector-scalar R-type instructions (SX_SLT)
`define OPCODE_VECTOR_SCALAR_I 7`b1110011  // Used by all vector-scalar I-type instructions (SX_SLTI)

typedef logic [`OPCODE_WIDTH-1:0] opcode_t;
typedef logic [`FUNCT3_WIDTH-1:0] funct3_t;
typedef logic [`FUNCT7_WIDTH-1:0] funct7_t;
typedef logic [11:0] imm12_t;

// alu instructions enum
typedef enum logic [4:0] {
    // immediate instructions
    ADDI,
    SLTI,
    XORI,
    ORI,
    ANDI,
    SLLI,
    SRLI,
    SRAI,

    // register instructions
    ADD,
    SUB,
    SLL,
    SLT,
    XOR,
    SRL,
    SRA,
    OR,
    AND,

    // compare instructions
    BEQ,
    BNE,
    BLT,
    BGE
} alu_instruction_t;

// warp state enum
typedef enum logic [2:0] {
    WARP_IDLE,
    WARP_FETCH,
    WARP_DECODE,
    WARP_REQUEST,
    WARP_WAIT,
    WARP_EXECUTE,
    WARP_UPDATE,
    WARP_DONE
} warp_state_t;

// fetcher state enum
typedef enum logic [1:0] {
    FETCHER_IDLE,
    FETCHER_FETCHING,
    FETCHER_DONE
} fetcher_state_t;

// lsu state enum
typedef enum logic [1:0] {
    LSU_IDLE,
    LSU_REQUESTING,
    LSU_WAITING,
    LSU_DONE
} lsu_state_t;

// sign extend function
function automatic data_t sign_extend(imm12_t imm12);
    data_t signed_imm12;
    if (imm12[11]) begin
        signed_imm12 = {{20{1'b1}}, imm12};
    end else begin
        signed_imm12 = {{20{1'b0}}, imm12};
    end
    return signed_imm12;
endfunction

// sign extend function for 13-bit immediate values
function automatic data_t sign_extend_13(logic[12:0] imm13);
    data_t signed_imm13;
    if (imm13[12]) begin
        signed_imm13 = {{19{1'b1}}, imm13};
    end else begin
        signed_imm13 = {{19{1'b0}}, imm13};
    end
    return signed_imm13;
endfunction

`endif // COMMON_SV
