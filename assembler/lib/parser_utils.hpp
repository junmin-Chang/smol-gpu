#pragma once

#include <string_view>
#include "common.hpp"
#include "token.hpp"
#include "error.hpp"

namespace as {

auto parse_num(std::string_view &source) -> std::expected<word_type, sim::Error>;

}
