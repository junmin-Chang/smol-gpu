#pragma once

#include <span>
#include "instructions.hpp"
#include "token.hpp"
#include "error.hpp"
#include <expected>

namespace as {

namespace parser {

struct WarpsDirective {
    std::uint32_t number;
};

struct BlocksDirective {
    std::uint32_t number;
};

using Operand = std::variant<token::Register, token::LabelRef, token::Immediate>;

struct Instruction {
    std::optional<std::string_view> label;
    sim::Mnemonic mnemonic;
};

using Line = std::variant<WarpsDirective, BlocksDirective, Instruction>;
auto line_to_str(const Line &line) -> std::string;

struct Program {
    std::int32_t threads{};
    std::int32_t warps{};
    std::vector<Instruction> instructions;
};

}

// The parsing function return nulloptr if an error has occured and push the error to
// the errors vector, otherwise they return the parsed value
class Parser {
  public:
    using Result = parser::Line;
    using Error = sim::Error;

    Parser() = delete;
    explicit Parser(const std::span<as::Token> &tokens) : tokens(tokens) {}

    auto chop() -> std::optional<as::Token>;
    [[nodiscard]] auto peek() const -> const as::Token *;

    template <typename ...T>
    auto expect() -> std::optional<as::Token> {
        auto next = chop();
        if(!next.has_value()) {
            throw_unexpected_eos(as::token::token_type_to_str<T...>());
            return std::nullopt;
        }

        if(!next->is_of_type<T...>()) {
            throw_unexpected_token(as::token::token_type_to_str<T...>(), *next);
            return std::nullopt;
        }

        return next;
    }

    auto parse_line() -> std::optional<Result>;
    auto parse_directive() -> std::optional<Result>;
    auto parse_instruction() -> std::optional<Result>;

    auto parse_rtype_instruction(sim::Mnemonic mnemonic) -> std::optional<parser::Instruction>;
    auto parse_itype_arithemtic_instruction(sim::Mnemonic mnemonic) -> std::optional<parser::Instruction>;
    auto parse_load_instruction(sim::Mnemonic mnemonic) -> std::optional<parser::Instruction>;
    auto parse_store_instruction(sim::Mnemonic mnemonic) -> std::optional<parser::Instruction>;
    auto parse_branch_instruction(sim::Mnemonic mnemonic) -> std::optional<parser::Instruction>;
    auto parse_jump_instruction(sim::Mnemonic mnemonic) -> std::optional<parser::Instruction>;
    auto parse_utype_instruction(sim::Mnemonic mnemonic) -> std::optional<parser::Instruction>;
    auto parse_sx_instruction(sim::Mnemonic mnemonic) -> std::optional<parser::Instruction>;

    void push_err(Error &&err);
    void push_err(std::string &&message, unsigned column);
    void throw_unexpected_token(std::string &&expected, const Token &unexpected);
    void throw_unexpected_eos(std::string &&expected);
    auto get_errors() -> std::span<Error> { return errors; }
    auto consume_errors() -> std::vector<Error> { return std::move(errors); }

  private:
    std::span<Token> tokens;
    std::vector<sim::Error> errors;
};

auto parse_line(const std::string_view &json) -> std::expected<Parser::Result, std::vector<Parser::Error>>;
} // namespace as
