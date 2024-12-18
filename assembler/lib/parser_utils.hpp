#pragma once

#include <string_view>
#include "token.hpp"
#include "error.hpp"

namespace as {

auto parse_num(std::string_view &source) -> std::expected<as::Immediate, sim::Error>;

}
