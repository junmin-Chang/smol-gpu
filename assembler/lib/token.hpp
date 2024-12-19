#pragma once
#include "common.hpp"
#include <string_view>
#include <variant>
#include <cstdint>
#include <verilated.h>

namespace as {

#define DEFINE_TOKEN_TYPE(name, ...)                                                                                   \
    struct name {                                                                                                      \
        __VA_ARGS__                                                                                                    \
        auto operator==(const name&) const -> bool = default;                                                    \
        auto operator!=(const name&) const -> bool = default;                                                    \
    };

/*
x0-x31 -> VECTOR
s0-s31 -> SCALAR
pc -> PC
*/
enum class RegisterType {
    VECTOR,
    SCALAR,
    PC
};

struct RegisterData {
    std::uint32_t register_number;
    RegisterType type;

    auto operator==(const RegisterData &other) const -> bool {
        return (register_number == other.register_number && type == other.type) || (type == RegisterType::PC && other.type == RegisterType::PC);
    }
    auto operator!=(const RegisterData &other) const -> bool = default;

    [[nodiscard]] auto is_scalar() const -> bool {
        return type == RegisterType::SCALAR;
    }

    [[nodiscard]] auto is_vector() const -> bool {
        return type == RegisterType::VECTOR;
    }

    [[nodiscard]] auto to_str() const -> std::string {
        switch (type) {
        case RegisterType::VECTOR:
            return std::format("x{}", register_number);
        case RegisterType::SCALAR:
            return std::format("s{}", register_number);
        case RegisterType::PC:
            return "pc";
        }
        return "unknown";
    }
};

DEFINE_TOKEN_TYPE(ThreadsDirective)
DEFINE_TOKEN_TYPE(WarpsDirective)
DEFINE_TOKEN_TYPE(Mnemonic, IData mnemonic;)
DEFINE_TOKEN_TYPE(Label, std::string_view name;)
DEFINE_TOKEN_TYPE(LabelRef, std::string_view label_name;)
DEFINE_TOKEN_TYPE(Immediate, std::int32_t value;)
DEFINE_TOKEN_TYPE(Register, RegisterData register_data;)
DEFINE_TOKEN_TYPE(Comma);
DEFINE_TOKEN_TYPE(Lparen);
DEFINE_TOKEN_TYPE(Rparen);

using TokenType = std::variant<ThreadsDirective, WarpsDirective, Mnemonic, Label, LabelRef, Immediate, Register, Comma, Lparen, Rparen>;

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

    template<typename T>
    [[nodiscard]] auto as() -> T & {
        return std::get<T>(token_type);
    }

    template<typename T>
    [[nodiscard]] auto as() const -> const T & {
        return std::get<T>(token_type);
    }

    [[nodiscard]] auto to_str() const -> std::string {
        return std::visit(overloaded{
                          [](const ThreadsDirective &) -> std::string { return ".threads"; },
                          [](const WarpsDirective &) -> std::string { return ".warps"; },
                          [](const Mnemonic &m) -> std::string { return std::to_string(m.mnemonic); },
                          [](const Label &l) -> std::string { return std::string(l.name); },
                          [](const LabelRef &lr) -> std::string { return std::string(lr.label_name); },
                          [](const Immediate &i) -> std::string { return std::to_string(i.value); },
                          [](const Register &r) -> std::string { return std::format("{}", r.register_data.to_str()); },
                          [](const Comma &) -> std::string { return "','"; },
                          [](const Lparen &) -> std::string { return "'('"; },
                          [](const Rparen &) -> std::string { return "')'"; },
                          },
                      token_type);
    }
};

}
