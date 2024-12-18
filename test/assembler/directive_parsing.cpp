#include "lexer.hpp"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

TEST_CASE("Lexing directives") {
    std::string_view input;

    SUBCASE("Threads Directive: correct input") {
        input = ".threads";
        const auto [tokens, errors] = as::collect_tokens(input);

        REQUIRE_EQ(tokens.size(), 1);
        REQUIRE_EQ(errors.size(), 0);
        REQUIRE(tokens[0].is_of_type<as::ThreadsDirective>());
    }

    SUBCASE("Threads Directive: incorrect input") {
        input = ".thread";
        const auto [tokens, errors] = as::collect_tokens(input);

        REQUIRE_EQ(tokens.size(), 0);
        REQUIRE_EQ(errors.size(), 1);
    }

    SUBCASE("Warps Directive: correct input") {
        input = ".warps";
        const auto [tokens, errors] = as::collect_tokens(input);

        REQUIRE_EQ(tokens.size(), 1);
        REQUIRE_EQ(errors.size(), 0);
        REQUIRE(tokens[0].is_of_type<as::WarpsDirective>());
    }

    SUBCASE("Warps Directive: incorrect input") {
        input = ".warp";
        const auto [tokens, errors] = as::collect_tokens(input);

        REQUIRE_EQ(tokens.size(), 0);
        REQUIRE_EQ(errors.size(), 1);
    }
}
