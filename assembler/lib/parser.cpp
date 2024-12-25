#include "parser.hpp"

namespace as {

    auto Parser::chop() -> std::optional<Token> {
        if (tokens.empty()) {
            return std::nullopt;
        }

        auto token = tokens.front();
        tokens = tokens.subspan(1);

        return token;
    }

    [[nodiscard]] auto Parser::peek() const -> const Token * {
        if (tokens.empty()) {
            return nullptr;
        }

        return &tokens.front();
    }



    void Parser::push_err(Error &&err) { errors.push_back(std::move(err)); }

    void Parser::push_err(std::string &&message, unsigned line, unsigned column) {
        errors.push_back(Error{.message = std::move(message)});
    }

    void Parser::throw_unexpected_token(std::string &&expected, const Token &unexpected) {
        push_err(std::format("Unexpected token: Expected {}, instead found {}", expected, to_string(unexpected.token_type)),
                 unexpected.row, unexpected.col);
    }

    void Parser::throw_unexpected_end_of_stream(std::string &&expected) {
        push_err(std::format("Unexpected end of stream: Expected {}", expected), 0, 0);
    }


}
