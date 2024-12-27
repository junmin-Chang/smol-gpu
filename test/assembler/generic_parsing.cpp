#include "instructions.hpp"
#include "lexer.hpp"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "assembler_helper.hpp"
#include "doctest.h"
#include <array>

TEST_CASE("Single character tokens") {
    SUBCASE("Token: Lparen") {
        std::string_view input = "(";
        const auto [tokens, errors] = as::collect_tokens(input);

        REQUIRE_EQ(tokens.size(), 1);
        REQUIRE_EQ(errors.size(), 0);
        REQUIRE(tokens[0].is_of_type<as::token::Lparen>());
    }

    SUBCASE("Token: Rparen") {
        std::string_view input = ")";
        const auto [tokens, errors] = as::collect_tokens(input);

        REQUIRE_EQ(tokens.size(), 1);
        REQUIRE_EQ(errors.size(), 0);
        REQUIRE(tokens[0].is_of_type<as::token::Rparen>());
    }

    SUBCASE("Token: Comma") {
        std::string_view input = ",";
        const auto [tokens, errors] = as::collect_tokens(input);

        REQUIRE_EQ(tokens.size(), 1);
        REQUIRE_EQ(errors.size(), 0);
        REQUIRE(tokens[0].is_of_type<as::token::Comma>());
    }
}

TEST_CASE("Multiple tokens: Valid use cases") {
    SUBCASE(".blocks 32") {
        std::string_view input = ".blocks 32";
        const auto [tokens, errors] = as::collect_tokens(input);

        REQUIRE_EQ(tokens.size(), 2);
        REQUIRE_EQ(errors.size(), 0);
        REQUIRE(tokens[0].is_of_type<as::token::BlocksDirective>());
        check_immediate(tokens[1], 32);
    }

    SUBCASE(".warps 0x4") {
        std::string_view input = ".warps 4";
        const auto [tokens, errors] = as::collect_tokens(input);

        REQUIRE_EQ(tokens.size(), 2);
        REQUIRE_EQ(errors.size(), 0);
        REQUIRE(tokens[0].is_of_type<as::token::WarpsDirective>());
        check_immediate(tokens[1], 4);
    }

    SUBCASE("add x1, x2, x3") {
        std::string_view input = "add x1, x2, x3";
        const auto [tokens, errors] = as::collect_tokens(input);

        REQUIRE_EQ(tokens.size(), 6);
        REQUIRE_EQ(errors.size(), 0);
        check_mnemonic(tokens[0], "add");
        check_reg(tokens[1], sim::RegisterType::VECTOR, 1);
        REQUIRE(tokens[2].is_of_type<as::token::Comma>());
        check_reg(tokens[3], sim::RegisterType::VECTOR, 2);
        REQUIRE(tokens[4].is_of_type<as::token::Comma>());
        check_reg(tokens[5], sim::RegisterType::VECTOR, 3);
    }

    SUBCASE("label: add s1, x2, pc") {
        std::string_view input = "label: add s1, x2, x3";
        const auto [tokens, errors] = as::collect_tokens(input);

        REQUIRE_EQ(tokens.size(), 7);
        REQUIRE_EQ(errors.size(), 0);
        check_label(tokens[0], "label");
        check_mnemonic(tokens[1], "add");
        check_reg(tokens[2], sim::RegisterType::SCALAR, 1);
        REQUIRE(tokens[3].is_of_type<as::token::Comma>());
        check_reg(tokens[4], sim::RegisterType::VECTOR, 2);
        REQUIRE(tokens[5].is_of_type<as::token::Comma>());
        check_reg(tokens[6], sim::RegisterType::VECTOR, 3);
    }

    SUBCASE("label123: jalr x1, some_label_ref,,, s31") {
        std::string_view input = "label123: jalr x1, some_label_ref,,, s31";
        const auto [tokens, errors] = as::collect_tokens(input);

        REQUIRE_EQ(tokens.size(), 9);
        REQUIRE_EQ(errors.size(), 0);
        check_label(tokens[0], "label123");
        check_mnemonic(tokens[1], "jalr");
        check_reg(tokens[2], sim::RegisterType::VECTOR, 1);
        REQUIRE(tokens[3].is_of_type<as::token::Comma>());
        check_label_ref(tokens[4], "some_label_ref");
        REQUIRE(tokens[5].is_of_type<as::token::Comma>());
        REQUIRE(tokens[6].is_of_type<as::token::Comma>());
        REQUIRE(tokens[7].is_of_type<as::token::Comma>());
        check_reg(tokens[8], sim::RegisterType::SCALAR, 31);
    }

    SUBCASE("label_90: sw x1, 0(x2)") {
        std::string_view input = "label_90: sw x1, 0(x2)";
        const auto [tokens, errors] = as::collect_tokens(input);

        REQUIRE_EQ(tokens.size(), 8);
        REQUIRE_EQ(errors.size(), 0);
        check_label(tokens[0], "label_90");
        check_mnemonic(tokens[1], "sw");
        check_reg(tokens[2], sim::RegisterType::VECTOR, 1);
        REQUIRE(tokens[3].is_of_type<as::token::Comma>());
        check_immediate(tokens[4], 0);
        REQUIRE(tokens[5].is_of_type<as::token::Lparen>());
        check_reg(tokens[6], sim::RegisterType::VECTOR, 2);
        REQUIRE(tokens[7].is_of_type<as::token::Rparen>());
    }
}
