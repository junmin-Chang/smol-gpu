#pragma once

#include <span>
#include "token.hpp"
#include "error.hpp"
#include <expected>

namespace as {

namespace parser {

using Instruction = std::variant<>;

struct Program {
    std::int32_t threads{};
    std::int32_t warps{};
    std::vector<Instruction> instructions{};
};

}

class Parser {
  public:
    using Result = parser::Program;
    using Error = sim::Error;

    Parser() = delete;
    explicit Parser(const std::span<as::Token> &tokens) : tokens(tokens) {}

    auto chop() -> std::optional<as::Token>;
    [[nodiscard]] auto peek() const -> const as::Token *;
    auto parse() -> std::optional<Result>;

    void push_err(Error &&err);
    void push_err(std::string &&message, unsigned line, unsigned column);
    void throw_unexpected_token(std::string &&expected, const Token &unexpected);
    void throw_unexpected_end_of_stream(std::string &&expected);
    auto get_errors() -> std::span<sim::Error>;

  private:
    std::span<Token> tokens;
    std::vector<sim::Error> errors;
};

auto parse(const std::string_view &json) -> std::expected<Parser::Result, std::vector<sim::Error>>;
} // namespace as
