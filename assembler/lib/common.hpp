#pragma once
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <variant>

namespace as {

using word_type = std::int32_t;

template <class... Ts> struct overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

constexpr auto is_numeric(char c, std::uint8_t base = 10) -> bool {
    if (base > 1 && base <= 10) {
        // bases 2-10
        return c >= '0' && c < '0' + base;
    } else if (base <= 10 + 26) {
        // bases 11-36 (number of digits + number of letters)
        return (c >= '0' && c <= '9') || (c >= 'A' && c < 'A' + base - 10) || (c >= 'a' && c < 'a' + base - 10);
    }
    return false;
}
constexpr auto is_lowercase_alphabetic(char c) -> bool { return (c >= 'a' && c <= 'z'); }
constexpr auto is_uppercase_alphabetic(char c) -> bool { return (c >= 'A' && c <= 'Z'); }
constexpr auto is_alphabetic(char c) -> bool { return is_lowercase_alphabetic(c) || is_uppercase_alphabetic(c); }
constexpr auto is_alphanumeric(char c, std::uint8_t base = 10) -> bool { return is_alphabetic(c) || is_numeric(c, base); }

constexpr auto is_label_char(char c) -> bool { return is_alphanumeric(c) || c == '_'; }

using num_type = std::variant<double, std::int64_t>;
constexpr auto as_double(num_type num) -> double {
    return std::visit([](auto n) -> double { return static_cast<double>(n); }, num);
}
constexpr auto as_int(num_type num) -> std::int64_t {
    return std::visit([](auto n) -> std::int64_t { return static_cast<std::int64_t>(n); }, num);
}

constexpr auto num_to_string(num_type num) -> std::string {
    return std::visit(overloaded{
                          [](double d) { return std::to_string(d); },
                          [](std::int64_t i) { return std::to_string(i); },
                      },
                      num);
}

}
