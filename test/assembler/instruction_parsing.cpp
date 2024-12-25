#include "instructions.hpp"
#include "lexer.hpp"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include <array>

TEST_CASE("Lexing instructions") {
    std::string_view input;

    const auto instructions = std::unordered_map<std::string_view, IData>{
        {"lui", sim::opcode::LUI},
        {"auipc", sim::opcode::AUIPC},
        {"addi", sim::opcode::ITYPE},
        {"add", sim::opcode::RTYPE},
        {"sub", sim::opcode::RTYPE},
        {"slli", sim::opcode::ITYPE},
        {"slti", sim::opcode::ITYPE},
        {"sltiu", sim::opcode::ITYPE},
        {"xori", sim::opcode::ITYPE},
        {"srli", sim::opcode::ITYPE},
        {"srai", sim::opcode::ITYPE},
        {"ori", sim::opcode::ITYPE},
        {"andi", sim::opcode::ITYPE},
        {"lb", sim::opcode::LOAD},
        {"lh", sim::opcode::LOAD},
        {"lw", sim::opcode::LOAD},
        {"sb", sim::opcode::STYPE},
        {"sh", sim::opcode::STYPE},
        {"sw", sim::opcode::STYPE},
        {"halt", sim::opcode::HALT},
        {"sx.slt", sim::opcode::SX_SLT},
        {"sx.slti", sim::opcode::SX_SLTI}
    };

    for (const auto &[instr, opcode] : instructions) {
        SUBCASE(std::format("Instruction: {}", instr).data()) {
            input = instr;
            const auto [tokens, errors] = as::collect_tokens(input);

            REQUIRE_EQ(tokens.size(), 1);
            REQUIRE_EQ(errors.size(), 0);
            REQUIRE(tokens[0].is_of_type<as::Mnemonic>());
            REQUIRE(std::get<as::Mnemonic>(tokens[0].token_type).mnemonic == opcode);
            REQUIRE(std::get<as::Mnemonic>(tokens[0].token_type).mnemonic == sim::opcode::str_to_opcode(instr));
        }
    }

    for (const auto &[instr, opcode] : instructions) {
        SUBCASE(std::format("Instruction: s.{}: Check scalar", instr).data()) {
            auto scalar_instr = std::format("s.{}", instr);
            std::println("Checking: {}", scalar_instr);
            input = scalar_instr;
            const auto [tokens, errors] = as::collect_tokens(input);

            REQUIRE_EQ(tokens.size(), 1);
            REQUIRE_EQ(errors.size(), 0);
            REQUIRE(tokens[0].is_of_type<as::Mnemonic>());
            REQUIRE(sim::opcode::is_scalar(std::get<as::Mnemonic>(tokens[0].token_type).mnemonic));
            REQUIRE(std::get<as::Mnemonic>(tokens[0].token_type).mnemonic == sim::opcode::str_to_opcode(scalar_instr));
}
    }

}

TEST_CASE("Lexing labels") {
    std::string_view input;

    const auto labels = std::array{
        "label:"sv,
        "label123:"sv,
        "label_123:"sv,
        "Label:"sv
    };

    for (const auto &label : labels) {
        SUBCASE(std::format("Label: correct input: '{}'", label).data()) {
            input = label;
            const auto [tokens, errors] = as::collect_tokens(input);

            REQUIRE_EQ(tokens.size(), 1);
            REQUIRE_EQ(errors.size(), 0);
            REQUIRE(tokens[0].is_of_type<as::Label>());
            REQUIRE(std::get<as::Label>(tokens[0].token_type).name == label.substr(0, label.size() - 1));
        }
    }


}
