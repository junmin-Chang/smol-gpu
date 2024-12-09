#include <print>
#include "Vgpu.h"

void tick(Vgpu& top) {
    top.clk = 0;
    top.eval();
    top.clk = 1;
    top.eval();
}

template <uint32_t address_width, uint32_t num_channels>
struct Memory {

    Vgpu* dut;
};

auto main() -> int {
    std::println("Hello from cpp!");

    Vgpu top{};
    return 0;
}
