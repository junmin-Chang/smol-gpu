#include "emitter.hpp"

namespace as {

auto translate_to_binary(const as::parser::Program& program) -> std::vector<sim::InstructionBits> {
    const auto len = program.instructions.size();
    auto machine_code = std::vector<sim::InstructionBits>(len);

    for(auto i = 0u; i < len; i++) {
        const auto instruction = program.instructions[i];
        auto instruction_bits = sim::InstructionBits{};

        const auto [opcode, funct3, funct7] = name_to_determinant(instruction.mnemonic.get_name());

        std::visit(as::overloaded{
            [&](const as::parser::ItypeOperands &operands) {
                instruction_bits = sim::instructions::create_itype_instruction(opcode, funct3, operands.rd, operands.rs1, (IData)operands.imm12.value);
            },
                [&](const as::parser::RtypeOperands &operands) {
                instruction_bits = sim::instructions::create_rtype_instruction(opcode, funct3, funct7, operands.rd, operands.rs1, operands.rs2);
            },
                [&](const as::parser::StypeOperands &operands) {
                instruction_bits = sim::instructions::create_stype_instruction(opcode, funct3, operands.rs1, operands.rs2, (IData)operands.imm12.value);
            },
                [&](const as::parser::UtypeOperands &operands) {
                instruction_bits = sim::instructions::create_utype_instruction(opcode, operands.rd, (IData)operands.imm20.value);
                }
        }, program.instructions[i].operands);

        machine_code[i] = instruction_bits;
    }

    return machine_code;
}

}
