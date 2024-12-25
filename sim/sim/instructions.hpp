#pragma once
#include "verilated.h"
#include "error.hpp"
#include <array>
#include <algorithm>
#include <bitset>

namespace sim {

// Opcodes
namespace opcode {
// if a define here is 6 bits, then the 7th leftmost bit defines whether it's a scalar or vector instruction
// 1 for scalar, 0 for vector
constexpr IData LUI      =  0b110111;         // Used by LUI (U-type)
constexpr IData AUIPC    =  0b010111;         // Used by AUIPC
constexpr IData ITYPE    =  0b010011;         // Used by ALU I-type instructions (ADDI, SLTI, XORI, ORI, ANDI, SLLI, SRLI, SRAI)
constexpr IData RTYPE    =  0b110011;         // Used by all R-type instructions (ADD, SUB, SLL, SLT, XOR, SRL, SRA)
constexpr IData LOAD     =  0b000011;         // Used by load instructions (LB, LH, LW)
constexpr IData STYPE    =  0b100011;         // Used by store instructions (SB, SH, SW)
// Jumps and branches can only be scalar instructions
constexpr IData JTYPE    = 0b1101111;         // Used by JAL
constexpr IData JALR     = 0b1100111;         // Used by JALR
constexpr IData BTYPE    = 0b1100011;         // Used by branch instructions (BEQ, BNE, BLT, BGE)
// Custom opcodes (also only scalar)
constexpr IData HALT     = 0b1111111;         // Used by HALT
constexpr IData SX_SLT   = 0b1111110;         // Used by SX_SLT
constexpr IData SX_SLTI  = 0b1111101;         // Used by SX_SLTI

constexpr auto opcodes = std::array{
    LUI,
    AUIPC,
    ITYPE,
    RTYPE,
    LOAD,
    STYPE,
    JTYPE,
    JALR,
    BTYPE,
    HALT,
    SX_SLT,
    SX_SLTI
};

constexpr auto is_scalar(IData opcode) -> bool {
    return (opcode & ((IData)1 << 6u)) != 0u;
}

constexpr auto is_vector(IData opcode) -> bool {
    return !is_scalar(opcode);
}

constexpr auto to_scalar(IData opcode) -> IData {
    return opcode | ((IData)1 << 6u);
}

constexpr auto str_to_opcode(std::string_view str) -> std::optional<IData> {
    auto is_scalar = str.starts_with("s.");
    if (is_scalar) {
        str.remove_prefix(2);
    }

    auto opcode = IData{};

    if (str == "lui") {
        opcode = LUI;
    } else if (str == "auipc") {
        opcode = AUIPC;
    } else if (str == "addi") {
        opcode = ITYPE;
    } else if (str == "add") {
        opcode = RTYPE;
    } else if (str == "sub") {
        opcode = RTYPE;
    } else if (str == "slli") {
        opcode = ITYPE;
    } else if (str == "slti") {
        opcode = ITYPE;
    } else if (str == "sltiu") {
        opcode = ITYPE;
    } else if (str == "xori") {
        opcode = ITYPE;
    } else if (str == "srli") {
        opcode = ITYPE;
    } else if (str == "srai") {
        opcode = ITYPE;
    } else if (str == "ori") {
        opcode = ITYPE;
    } else if (str == "andi") {
        opcode = ITYPE;
    } else if (str == "lb") {
        opcode = LOAD;
    } else if (str == "lh") {
        opcode = LOAD;
    } else if (str == "lw") {
        opcode = LOAD;
    } else if (str == "sb") {
        opcode = STYPE;
    } else if (str == "sh") {
        opcode = STYPE;
    } else if (str == "sw") {
        opcode = STYPE;
    } else if (str == "beq") {
        opcode = BTYPE;
    } else if (str == "bne") {
        opcode = BTYPE;
    } else if (str == "blt") {
        opcode = BTYPE;
    } else if (str == "bge") {
        opcode = BTYPE;
    } else if (str == "jal") {
        opcode = JTYPE;
    } else if (str == "jalr") {
        opcode = JALR;
    } else if (str == "halt") {
        opcode = HALT;
    } else if (str == "sx.slt") {
        opcode = SX_SLT;
    } else if (str == "sx.slti") {
        opcode = SX_SLTI;
    } else {
        return std::nullopt;
    }

    if (is_scalar) {
        opcode = to_scalar(opcode);
    }

    return opcode;
}
}

// Funct3
namespace funct3 {
// I-type
constexpr IData ADDI            = 0b000;
constexpr IData SLTI            = 0b010;
constexpr IData XORI            = 0b100;
constexpr IData ORI             = 0b110;
constexpr IData ANDI            = 0b111;
constexpr IData SLLI            = 0b001;
constexpr IData SRLI            = 0b101;
constexpr IData SRAI            = 0b101;
// R-type
constexpr IData ADD             = 0b000;
constexpr IData SUB             = 0b000;
constexpr IData SLL             = 0b001;
constexpr IData SLT             = 0b010;
constexpr IData XOR             = 0b100;
constexpr IData SRL             = 0b101;
constexpr IData SRA             = 0b101;
constexpr IData OR              = 0b110;
constexpr IData AND             = 0b111;
// Load
constexpr IData LB              = 0b000;
constexpr IData LH              = 0b001;
constexpr IData LW              = 0b010;
// Store
constexpr IData SB              = 0b000;
constexpr IData SH              = 0b001;
constexpr IData SW              = 0b010;
// JALR
constexpr IData JALR            = 0b000;
// B-type
constexpr IData BEQ             = 0b000;
constexpr IData BNE             = 0b001;
constexpr IData BLT             = 0b100;
constexpr IData BGE             = 0b101;

constexpr auto funct3s = std::array<IData, 28>{
    ADDI,
    SLTI,
    XORI,
    ORI,
    ANDI,
    SLLI,
    SRLI,
    SRAI,
    ADD,
    SUB,
    SLL,
    SLT,
    XOR,
    SRL,
    SRA,
    OR,
    AND,
    LB,
    LH,
    LW,
    SB,
    SH,
    SW,
    JALR,
    BEQ,
    BNE,
    BLT,
    BGE
};
}

// Funct7
namespace funct7 {
// I-type
constexpr IData SLLI            = 0b0000000;
constexpr IData SRLI            = 0b0000000;
constexpr IData SRAI            = 0b0100000;
// R-type
constexpr IData ADD             = 0b0000000;
constexpr IData SUB             = 0b0100000;
constexpr IData SLL             = 0b0000000;
constexpr IData SLT             = 0b0000000;
constexpr IData XOR             = 0b0000000;
constexpr IData SRL             = 0b0000000;
constexpr IData SRA             = 0b0100000;
constexpr IData OR              = 0b0000000;
constexpr IData AND             = 0b0000000;

constexpr auto funct7s = std::array<IData, 12>{
    SLLI,
    SRLI,
    SRAI,
    ADD,
    SUB,
    SLL,
    SLT,
    XOR,
    SRL,
    SRA,
    OR,
    AND
};
}

// Helper function for validating register numbers
inline void validate_register(IData reg) {
    assert_or_err(reg < 32, std::format("Invalid register number: '{}', expected values 0-31.", reg));
}

// Helper function for validating instruction IDs
// used for validating opcodes, funct3s, and funct7s
// possible_ids should be a std::array<IData, N>
constexpr void validate_instr_id(const std::string_view id_name, const IData id, const auto &possible_ids) {
    const auto bit_string = std::bitset<7>(id).to_string();
    const auto found = std::find(possible_ids.begin(), possible_ids.end(), id);
    assert_or_err(found != possible_ids.end(), std::format("Unknown {}: '0b{}'", id_name, bit_string));
}

struct Instruction {
    IData bits{};

    Instruction() = default;
    Instruction(IData bits) : bits(bits) {}

    operator IData() {
        return bits;
    }

    constexpr auto set_opcode(IData opcode) -> Instruction& {
        validate_instr_id("opcode", opcode, opcode::opcodes);
        bits |= opcode;
        return *this;
    }

    constexpr auto set_rd(IData rd) -> Instruction& {
        validate_register(rd);
        bits |= rd << 7u;
        return *this;
    }

    constexpr auto set_funct3(IData funct3) -> Instruction& {
        validate_instr_id("funct3", funct3, funct3::funct3s);
        bits |= funct3 << 12u;
        return *this;
    }

    constexpr auto set_rs1(IData rs1) -> Instruction& {
        validate_register(rs1);
        bits |= rs1 << 15u;
        return *this;
    }

    constexpr auto set_rs2(IData rs2) -> Instruction& {
        validate_register(rs2);
        bits |= rs2 << 20u;
        return *this;
    }

    constexpr auto set_imm12(IData imm) -> Instruction& {
        assert_or_err(imm < 4096, std::format("Invalid immediate: '{}', expected 12-bit.", imm));
        bits |= imm << 20u;
        return *this;
    }

    constexpr auto set_funct7(IData funct7) -> Instruction& {
        validate_instr_id("funct7", funct7, funct7::funct7s);
        bits |= funct7 << 25u;
        return *this;
    }

    constexpr auto set_imm20(IData imm20) -> Instruction& {
        assert_or_err(imm20 < 1048576, std::format("Invalid immediate: '{}', expected 20-bit.", imm20));
        bits |= imm20 << 12u;
        return *this;
    }

    constexpr auto set_imm21(IData imm21) -> Instruction& {
        assert_or_err(imm21 < 2097152, std::format("Invalid immediate: '{}', expected 21-bit.", imm21));

        auto imm_j = std::bitset<32>{};
        auto imm = std::bitset<21>(imm21);

        imm_j[31] = imm[20];
        for(auto i = 21u; i <= 30u; i++) {
            imm_j[i] = imm[i - 20];
        }
        imm_j[20] = imm[11];
        for(auto i = 12u; i <= 19u; i++) {
            imm_j[i] = imm[i];
        }
        imm_j[0] = 0;

        bits |= imm_j.to_ulong();
        return *this;
    }

    constexpr auto make_scalar() -> Instruction& {
        bits |= (IData)1 << 6u;
        return *this;
    }

    constexpr auto make_vector() -> Instruction& {
        bits &= ~((IData)1 << 6u);
        return *this;
    }
};

namespace instructions {

// Helper functions for creating instructions
constexpr auto create_utype_instruction(IData opcode, IData rd, IData imm20) -> Instruction {
    return Instruction().set_opcode(opcode).set_rd(rd).set_imm20(imm20);
}
constexpr auto create_itype_instruction(IData opcode, IData funct3, IData rd, IData rs1, IData imm12) -> Instruction {
    return Instruction().set_opcode(opcode).set_funct3(funct3).set_rd(rd).set_rs1(rs1).set_imm12(imm12);
}
constexpr auto create_itype_shift_instruction(IData opcode, IData funct3, IData funct7, IData rd, IData rs1, IData imm12) -> Instruction {
    assert_or_err(imm12 < 32, std::format("Invalid immediate: '{}', expected 5-bit immediate in shift instruction.", imm12));
    return Instruction().set_opcode(opcode).set_funct3(funct3).set_funct7(funct7).set_rd(rd).set_rs1(rs1).set_imm12(imm12);
}
constexpr auto create_rtype_instruction(IData opcode, IData funct3, IData funct7, IData rd, IData rs1, IData rs2) -> Instruction {
    return Instruction().set_opcode(opcode).set_funct3(funct3).set_funct7(funct7).set_rd(rd).set_rs1(rs1).set_rs2(rs2);
}
constexpr auto create_jtype_instruction(IData opcode, IData rd, IData imm21) -> Instruction {
    return Instruction().set_opcode(opcode).set_rd(rd).set_imm21(imm21);
}
constexpr auto create_btype_instruction(IData opcode, IData funct3, IData rs1, IData rs2, IData imm12) -> Instruction {
    assert_or_err(false, "Not yet implemented (need to properly set the immediate)");
    return Instruction().set_opcode(opcode).set_funct3(funct3).set_rs1(rs1).set_rs2(rs2);
}
constexpr auto create_stype_instruction(IData opcode, IData funct3, IData rs1, IData rs2, IData imm12) -> Instruction {
    return Instruction().set_opcode(opcode).set_funct3(funct3).set_rs1(rs1).set_rs2(rs2).set_imm12(imm12);
}

// Instruction constructors

// U-type
constexpr auto lui(IData rd, IData imm20) -> Instruction {
    return create_utype_instruction(opcode::LUI, rd, imm20);
}
constexpr auto auipc(IData rd, IData imm20) -> Instruction {
    return create_utype_instruction(opcode::AUIPC, rd, imm20);
}

// I-type
constexpr auto addi(IData rd, IData rs1, IData imm12) -> Instruction {
    return create_itype_instruction(opcode::ITYPE, funct3::ADDI, rd, rs1, imm12);
}
constexpr auto slti(IData rd, IData rs1, IData imm12) -> Instruction {
    return create_itype_instruction(opcode::ITYPE, funct3::SLTI, rd, rs1, imm12);
}
constexpr auto xori(IData rd, IData rs1, IData imm) -> Instruction {
    return create_itype_instruction(opcode::ITYPE, funct3::XORI,  rd, rs1, imm);
}
constexpr auto ori(IData rd, IData rs1, IData imm) -> Instruction {
    return create_itype_instruction(opcode::ITYPE, funct3::ORI, rd, rs1, imm);
}
constexpr auto andi(IData rd, IData rs1, IData imm) -> Instruction {
    return create_itype_instruction(opcode::ITYPE, funct3::ANDI, rd, rs1, imm);
}
constexpr auto slli(IData rd, IData rs1, IData imm) -> Instruction {
    return create_itype_shift_instruction(opcode::ITYPE, funct3::SLLI, funct7::SLLI, rd, rs1, imm);
}
constexpr auto srli(IData rd, IData rs1, IData imm) -> Instruction {
    return create_itype_shift_instruction(opcode::ITYPE, funct3::SRLI, funct7::SRLI, rd, rs1, imm);
}
constexpr auto srai(IData rd, IData rs1, IData imm) -> Instruction {
    return create_itype_shift_instruction(opcode::ITYPE, funct3::SRAI, funct7::SRAI, rd, rs1, imm);
}

// R-type
constexpr auto add(IData rd, IData rs1, IData rs2) -> Instruction {
    return create_rtype_instruction(opcode::RTYPE, funct3::ADD, funct7::ADD, rd, rs1, rs2);
}
constexpr auto sub(IData rd, IData rs1, IData rs2) -> Instruction {
    return create_rtype_instruction(opcode::RTYPE, funct3::SUB, funct7::SUB, rd, rs1, rs2);
}
constexpr auto sll(IData rd, IData rs1, IData rs2) -> Instruction {
    return create_rtype_instruction(opcode::RTYPE, funct3::SLL, funct7::SLL, rd, rs1, rs2);
}
constexpr auto slt(IData rd, IData rs1, IData rs2) -> Instruction {
    return create_rtype_instruction(opcode::RTYPE, funct3::SLT, funct7::SLT, rd, rs1, rs2);
}
constexpr auto xor_(IData rd, IData rs1, IData rs2) -> Instruction {
    return create_rtype_instruction(opcode::RTYPE, funct3::XOR, funct7::XOR, rd, rs1, rs2);
}
constexpr auto srl(IData rd, IData rs1, IData rs2) -> Instruction {
    return create_rtype_instruction(opcode::RTYPE, funct3::SRL, funct7::SRL, rd, rs1, rs2);
}
constexpr auto sra(IData rd, IData rs1, IData rs2) -> Instruction {
    return create_rtype_instruction(opcode::RTYPE, funct3::SRA, funct7::SRA, rd, rs1, rs2);
}
constexpr auto or_(IData rd, IData rs1, IData rs2) -> Instruction {
    return create_rtype_instruction(opcode::RTYPE, funct3::OR, funct7::OR, rd, rs1, rs2);
}
constexpr auto and_(IData rd, IData rs1, IData rs2) -> Instruction {
    return create_rtype_instruction(opcode::RTYPE, funct3::AND, funct7::AND, rd, rs1, rs2);
}

// Load
constexpr auto lb(IData rd, IData rs1, IData imm12) -> Instruction {
    return create_itype_instruction(opcode::LOAD, funct3::LB, rd, rs1, imm12);
}
constexpr auto lh(IData rd, IData rs1, IData imm12) -> Instruction {
    return create_itype_instruction(opcode::LOAD, funct3::LH, rd, rs1, imm12);
}
constexpr auto lw(IData rd, IData rs1, IData imm12) -> Instruction {
    return create_itype_instruction(opcode::LOAD, funct3::LW, rd, rs1, imm12);
}

// Store
constexpr auto sb(IData rs1, IData rs2, IData imm12) -> Instruction {
    return create_stype_instruction(opcode::STYPE, funct3::SB, rs1, rs2, imm12);
}
constexpr auto sh(IData rs1, IData rs2, IData imm12) -> Instruction {
    return create_stype_instruction(opcode::STYPE, funct3::SH, rs1, rs2, imm12);
}
constexpr auto sw(IData rs1, IData rs2, IData imm12) -> Instruction {
    return create_stype_instruction(opcode::STYPE, funct3::SW, rs1, rs2, imm12);
}

// J-type
constexpr auto jal(IData rd, IData imm21) -> Instruction {
    return create_jtype_instruction(opcode::JTYPE, rd, imm21);
}

// JALR
constexpr auto jalr(IData rd, IData rs1, IData imm12) -> Instruction {
    return create_itype_instruction(opcode::JALR, funct3::JALR, rd, rs1, imm12);
}

// B-type
constexpr auto beq(IData rs1, IData rs2, IData imm12) -> Instruction {
    return create_btype_instruction(opcode::BTYPE, funct3::BEQ, rs1, rs2, imm12);
}
constexpr auto bne(IData rs1, IData rs2, IData imm12) -> Instruction {
    return create_btype_instruction(opcode::BTYPE, funct3::BNE, rs1, rs2, imm12);
}
constexpr auto blt(IData rs1, IData rs2, IData imm12) -> Instruction {
    return create_btype_instruction(opcode::BTYPE, funct3::BLT, rs1, rs2, imm12);
}
constexpr auto bge(IData rs1, IData rs2, IData imm12) -> Instruction {
    return create_btype_instruction(opcode::BTYPE, funct3::BGE, rs1, rs2, imm12);
}

// Custom opcodes
constexpr auto halt() -> Instruction {
    return Instruction().set_opcode(opcode::HALT);
}
constexpr auto sx_slt(IData rd, IData rs1, IData rs2) -> Instruction {
    return create_rtype_instruction(opcode::SX_SLT, funct3::SLT, funct7::SLT, rd, rs1, rs2);
}
constexpr auto sx_slti(IData rd, IData rs1, IData imm12) -> Instruction {
    return create_itype_instruction(opcode::SX_SLTI, funct3::SLTI, rd, rs1, imm12);
}




}

}
