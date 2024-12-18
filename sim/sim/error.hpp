#pragma once
#include <print>
#include <string_view>
#include <string>
#include <cassert>
#include <expected>

namespace sim {

using Error = std::string;

inline void error(const std::string_view message) {
    std::println(stderr, "Error: {}.", message);
    assert(false);
}

#ifndef NDEBUG
inline void assert_or_err(bool condition, const std::string_view message) {
    if (!condition) {
        error(message);
    }
}
#else
inline void assert_or_err(bool condition, const std::string_view message) {
    // no-op in release
}
#endif

template<typename T, typename E>
auto unwrap(std::expected<T, E>&& expected) -> T {
    if (!expected.has_value()) {
        error(expected.error());
        std::exit(1);
    }
    return std::move(*expected);
}
}
