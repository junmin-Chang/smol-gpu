#include "lexer.hpp"
#include "common.hpp"
#include "parser_utils.hpp"

namespace as {

auto Lexer::chop() -> char {
    auto result = source.front();
    source.remove_prefix(1);
    column_number++;
    return result;
}

auto Lexer::chop_while(const std::function<bool(char)> &predicate) -> std::string_view {
    auto i = 0u;
    while (i < source.size() && predicate(source[i])) {
        i++;
        column_number++;
    }

    auto result = source.substr(0, i);
    source.remove_prefix(i);
    return result;
}

auto Lexer::peek() -> std::optional<char> {
    if (source.empty()) {
        return std::nullopt;
    }
    return source.front();
}

void Lexer::trim_whitespace() {
    while (auto c = peek()) {
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            chop();
        } else {
            break;
        }
    }
}

auto Lexer::make_error(const std::string_view message) -> Error {
    return std::format("Error:{}: {}.", column_number, message);
}

auto Lexer::parse_directive() -> std::expected<Token, Error> {
    auto keyword = chop_while(is_alphabetic);

    if (keyword == "threads") {
        return Token{ThreadsDirective{}, column_number};
    }

    if (keyword == "warps") {
        return Token{WarpsDirective{}, column_number};
    }

    return std::unexpected(make_error(std::format("Unexpected keyword '{}'", keyword)));
}

auto Lexer::parse_number() -> std::expected<Token, Error> {
    const auto starting_col = column_number;
    const auto source_size_before = source.size();
    auto number = parse_num(source);
    if (!number.has_value()) {
        auto error = number.error();
        return std::unexpected(error);
    }
    column_number += source_size_before - source.size();
    return Token{*number, starting_col};
}

auto Lexer::next_token() -> std::optional<std::expected<Token, Error>> {
    trim_whitespace();

    const auto first_char_column = column_number;

    if (source.empty()) {
        return std::nullopt;
    }

    const char c = *peek();

    if (is_numeric(c)) {
        return parse_number();
    }

    /*if (is_alphabetic(c)) {*/
    /*    return parse_keyword();*/
    /*}*/

    chop();

    /*switch (c) {*/
    /*case '{':*/
    /*    return as::Token{.token_type = as::LBrace{}, .row = line_number, .col = first_char_column};*/
    /*case '}':*/
    /*    return as::Token{.token_type = as::RBrace{}, .row = line_number, .col = first_char_column};*/
    /*case '[':*/
    /*    return as::Token{.token_type = as::LBracket{}, .row = line_number, .col = first_char_column};*/
    /*case ']':*/
    /*    return as::Token{.token_type = as::RBracket{}, .row = line_number, .col = first_char_column};*/
    /*case ',':*/
    /*    return as::Token{.token_type = as::Comma{}, .row = line_number, .col = first_char_column};*/
    /*case ':':*/
    /*    return as::Token{.token_type = as::Colon{}, .row = line_number, .col = first_char_column};*/
    /*};*/
    return std::unexpected(make_error(std::format("Unexpected character '{}'", c)));
}

auto collect_tokens(const std::string_view source) -> std::pair<std::vector<Token>, std::vector<sim::Error>> {
    auto tokens = std::vector<Token>{};
    auto errors = std::vector<sim::Error>{};

    for (const auto &next_token : Lexer(source)) {
        if (next_token.has_value()) {
            tokens.push_back(next_token.value());
        } else {
            errors.push_back(next_token.error());
        }
    }

    return {tokens, errors};
}

}
