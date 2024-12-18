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

using TokenType = std::variant<ThreadsDirective, WarpsDirective, Mnemonic, Label, LabelRef, Immediate, Register, Comma>;

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
                      },
                      token_type);
    }
};

}
