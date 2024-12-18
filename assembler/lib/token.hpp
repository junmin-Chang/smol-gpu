#pragma once
#include <string_view>
#include <variant>
#include <cstdint>

namespace as {

#define DEFINE_TOKEN_TYPE(name, ...)                                                                                   \
    struct name {                                                                                                      \
        __VA_ARGS__                                                                                                    \
        auto operator==(const name&) const -> bool = default;                                                    \
        auto operator!=(const name&) const -> bool = default;                                                    \
    };

enum class RegisterType {
    VECTOR,
    SCALAR,
    PC
};

struct RegisterData {
    std::uint32_t register_number;
    RegisterType type;

    auto operator==(const RegisterData &other) const -> bool = default;
    auto operator!=(const RegisterData &other) const -> bool = default;
};

DEFINE_TOKEN_TYPE(ThreadsDirective)
DEFINE_TOKEN_TYPE(WarpsDirective)
DEFINE_TOKEN_TYPE(Mnemonic, std::string_view mnemonic;)
DEFINE_TOKEN_TYPE(Label, std::string_view name;)
DEFINE_TOKEN_TYPE(LabelRef, std::string_view label_name;)
DEFINE_TOKEN_TYPE(Immediate, std::int32_t value;)
DEFINE_TOKEN_TYPE(Register, RegisterData register_data;)
DEFINE_TOKEN_TYPE(Comma);

using TokenType = std::variant<ThreadsDirective, WarpsDirective, Mnemonic, Label, Immediate, Register, Comma>;

struct Token {
    TokenType token_type;
    std::uint32_t col;

    auto operator==(const Token &other) const -> bool {
        return token_type == other.token_type && col == other.col;
    }

    template<typename T>
    [[nodiscard]] auto is_of_type() const -> bool {
        return std::holds_alternative<T>(token_type);
    }
};

}
