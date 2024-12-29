#include <Vgpu.h>
#include <print>
#include <fstream>
#include <expected>
#include <format>
#include "common.hpp"
#include "instructions.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "error.hpp"
#include "sim.hpp"
#include <vector>
#include <cstdint>
#include <variant>
#include <string_view>
#include <algorithm>

auto open_file(const std::string_view filename) -> std::expected<std::ifstream, std::string_view> {
    auto file = std::ifstream{filename.data()};
    if (!file.is_open()) {
        return std::unexpected{std::format("Failed to open file: {}", filename)};
    }
    return file;
}

auto get_lines(std::ifstream& file) -> std::vector<std::string> {
    auto lines = std::vector<std::string>{};
    for (auto line = std::string{}; std::getline(file, line);) {
        lines.push_back(line);
    }
    return lines;
}

auto trim_line(std::string_view& line) -> std::string_view {
    while (!line.empty() && as::is_whitespace(line.front())) {
        line.remove_prefix(1);
    }
    while (!line.empty() && as::is_whitespace(line.front())) {
        line.remove_suffix(1);
    }
    return line;
}

constexpr auto is_whitespace(std::string_view str) -> bool {
    return str.empty() || std::all_of(str.begin(), str.end(), [](char c) { return as::is_whitespace(c); });
}

auto parse_program(const std::span<const std::string> lines) -> std::expected<as::parser::Program, std::vector<sim::Error>> {
    auto program = as::parser::Program{};
    auto errors = std::vector<sim::Error>{};

    std::optional<std::uint32_t> block_count{};
    std::optional<std::uint32_t> warp_count{};

    auto line_nr = 0u;
    auto instr_count = 0u;

    for(const auto& line : lines) {
        std::println("Parsing line: {}", line);
        line_nr++;

        // Tokenize
        auto [tokens, lexer_errors] = as::collect_tokens(line);
        if (!lexer_errors.empty()) {
            for (auto error : lexer_errors) {
                errors.push_back(error.with_line(line_nr));
            }
        }

        for (const auto& token : tokens) {
            std::println("Token: {}", token.to_str());
        }

        // Skip empty lines
        if (tokens.empty()) {
            continue;
        }

        const auto output = as::parse_line(tokens);
        if(!output.has_value()) {
            for (auto err : output.error()) {
                errors.push_back(err.with_line(line_nr));
            }
            continue;
        }

        const auto& val = output.value();
        std::visit(as::overloaded{
                [&](const as::parser::JustLabel& label) {
                    program.label_mappings[label.label.name] = instr_count;
                },
                [&](const as::parser::Instruction& instr) {
                    program.instructions.push_back(instr);
                    instr_count++;
                },
                [&](const as::parser::BlocksDirective& block) {
                    if(block_count.has_value()) {
                        errors.push_back(sim::Error{"Duplicate blocks directive", 0, line_nr});
                    }
                    block_count = block.number;
                },
                [&](const as::parser::WarpsDirective& warp) {
                    if(warp_count.has_value()) {
                        errors.push_back(sim::Error{"Duplicate warps directive", 0, line_nr});
                    }
                    warp_count = warp.number;
                },
        }, val);
    }

    program.blocks = block_count.value_or(1);
    program.warps = warp_count.value_or(1);

    if (!errors.empty()) {
        return std::unexpected{errors};
    }

    return program;
}

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
            }
        }, program.instructions[i].operands);

        machine_code[i] = instruction_bits;
    }

    return machine_code;
}

auto main(int argc, char** argv) -> int {
    if (argc < 2) {
        std::println("Usage: {} <input file>", argv[0]);
        return 1;
    }

    auto input_file = sim::unwrap(open_file(argv[1]));
    const auto lines = get_lines(input_file);

    input_file.close();

    auto program_or_err = parse_program(lines);

    if (!program_or_err.has_value()) {
        for (const auto& error : program_or_err.error()) {
            sim::print_error(error);
        }
        return 1;
    }

    const auto& [blocks, warps, instructions, label_mappings] = program_or_err.value();

    std::println("\nSuccesfully parsed the entire file!");
    std::println("Warps: {}, Blocks: {}", warps, blocks);
    std::println("Parsed {} instructions:", instructions.size());
    auto i = 0u;
    for (const auto& instr : instructions) {
        std::println("{:3}: {}", i, instr.to_str());
        i++;
    }

    const auto machine_code = translate_to_binary(*program_or_err);
    Vgpu top{};

    constexpr auto num_channels = 8;
    constexpr auto mem_cells_count = 2048;
    auto data_mem = sim::make_data_memory<mem_cells_count, num_channels>(&top);
    auto instruction_mem = sim::make_instruction_memory<mem_cells_count, num_channels>(&top);
    
    for (auto i = 0u; i < machine_code.size(); i++) {
        instruction_mem.memory[i] = machine_code[i].bits;
    }

    sim::set_kernel_config(top, 0, 0, blocks, warps);

    // Run simulation
    auto done = sim::simulate(top, instruction_mem, data_mem, 200);

    if(!done) {
        std::println("Simulation failed!");
        return 1;
    }

    // Optionally, print data memory content
    data_mem.print_memory(0, 10);


    return 0;
}
