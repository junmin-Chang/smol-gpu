#include <print>
#include <fstream>
#include <expected>
#include <format>
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
    while (!line.empty() && std::isspace(line.front())) {
        line.remove_prefix(1);
    }
    while (!line.empty() && std::isspace(line.back())) {
        line.remove_suffix(1);
    }
    return line;
}

auto main(int argc, char** argv) -> int {
    std::string_view str = "label:";

    const auto& [tokens, errors] = as::collect_tokens(str);

    for (const auto &error : errors) {
        std::println("Error: {}", error);
    }

    for (const auto &token : tokens) {
        std::println("{}", token.to_str());
    }

    /*if (argc < 3) {*/
    /*    std::println("Usage: {} <input file> <output_file>", argv[0]);*/
    /*    return 1;*/
    /*}*/
    /**/
    /*auto input_file = sim::unwrap(open_file(argv[1]));*/
    /*const auto lines = get_lines(input_file);*/
    /**/
    /*auto output_file = std::ofstream{argv[2]};*/
    /*if(!output_file.is_open()) {*/
    /*    sim::error("Failed to open output file.");*/
    /*}*/
    /**/
    /*std::optional<Threads> thread_count{};*/
    /*std::optional<Warps> warp_count{};*/
    /**/
    /*for(const auto& line : lines) {*/
    /*    if (line.empty()) {*/
    /*        continue;*/
    /*    }*/
    /**/
    /*    const auto output = parse_line(line);*/
    /*    if(!output.has_value()) {*/
    /*        sim::error(output.error());*/
    /*    }*/
    /**/
    /*    const auto& val = output.value();*/
    /*    std::visit([&](const auto& val) {*/
    /*        using T = std::decay_t<decltype(val)>;*/
    /*        if constexpr (std::is_same_v<T, Instruction>) {*/
    /*            sim::error("Instruction not supported.");*/
    /*        } else if constexpr (std::is_same_v<T, Threads>) {*/
    /*            if(thread_count.has_value()) {*/
    /*                sim::error("Thread count defined multiple times.");*/
    /*            }*/
    /*            output_file << val.count;*/
    /*        } else if constexpr (std::is_same_v<T, Warps>) {*/
    /*            if(warp_count.has_value()) {*/
    /*                sim::error("Warp count defined multiple times.");*/
    /*            }*/
    /*            output_file << val.count;*/
    /*        } else if constexpr (std::is_same_v<T, Data>) {*/
    /*            sim::error("Data not supported.");*/
    /*        }*/
    /*    }, val);*/
    /**/
    /**/
    /**/
    /*}*/
    /**/
    /*output_file.close();*/

    return 0;
}
