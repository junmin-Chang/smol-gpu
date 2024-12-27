#include <print>
#include <fstream>
#include <expected>
#include <format>
#include "common.hpp"
#include "parser.hpp"
#include "parser_utils.hpp"
#include "error.hpp"
#include <vector>
#include <cstdint>
#include <variant>
#include <string_view>
#include "lexer.hpp"

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

auto parse_program(const std::span<const std::string> lines) -> std::expected<as::parser::Program, std::vector<sim::Error>> {
    auto program = as::parser::Program{};
    auto errors = std::vector<sim::Error>{};

    std::optional<std::uint32_t> block_count{};
    std::optional<std::uint32_t> warp_count{};

    auto line_nr = 0u;
    auto instr_count = 0u;

    for(const auto& line : lines) {
        line_nr++;
        if (line.empty()) {
            continue;
        }

        const auto output = as::parse_line(line);
        if(!output.has_value()) {
            for (auto err : output.error()) {
                errors.push_back(err.with_line(line_nr));
            }
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

    return program;
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

    std::println("Succesfully parsed the entire file!");
    std::println("Warps: {}, Blocks: {}", warps, blocks);
    std::println("Parsed {} instructions:", instructions.size());
    auto i = 0u;
    for (const auto& instr : instructions) {
        std::println("{:3}: {}", i, instr.to_str());
        i++;
    }

    return 0;
}
