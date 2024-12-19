#pragma once
#include "doctest.h"
#include "instructions.hpp"
#include "token.hpp"

inline void check_reg(const as::Token& token, as::RegisterType type, std::uint32_t reg_num) {
    const auto data = as::RegisterData{.register_number = reg_num, .type = type};
    REQUIRE(token.is_of_type<as::Register>());
    REQUIRE_EQ(token.as<as::Register>().register_data, data);
}

inline void check_label(const as::Token& token, std::string_view label_name) {
    REQUIRE(token.is_of_type<as::Label>());
    REQUIRE_EQ(token.as<as::Label>().name, label_name);
}

inline void check_label_ref(const as::Token& token, std::string_view label_name) {
    REQUIRE(token.is_of_type<as::LabelRef>());
    REQUIRE_EQ(token.as<as::LabelRef>().label_name, label_name);
}

inline void check_immediate(const as::Token& token, std::int32_t value) {
    REQUIRE(token.is_of_type<as::Immediate>());
    REQUIRE_EQ(token.as<as::Immediate>().value, value);
}

inline void check_mnemonic(const as::Token& token, const std::string_view mnemonic) {
    REQUIRE(token.is_of_type<as::Mnemonic>());
    REQUIRE_EQ(token.as<as::Mnemonic>().mnemonic, sim::opcode::str_to_opcode(mnemonic));
}
