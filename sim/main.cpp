#include <print>
#include "sim.hpp"

auto main() -> int {
    std::println("Hello from cpp!");

    Vgpu top{};

    constexpr auto num_channels = 8;
    constexpr auto mem_cells_count = 2048;
    auto data_mem = sim::make_data_memory<mem_cells_count, num_channels>(&top);
    auto instruction_mem = sim::make_instruction_memory<mem_cells_count, num_channels>(&top);

    instruction_mem.push_instruction(sim::addi(5, 1 ,0));
    instruction_mem.push_instruction(sim::sw(5, 1, 0));
    instruction_mem.push_instruction(sim::halt());

    // Prepare kernel configuration
    sim::set_kernel_config(top, 0, 0, 1, 1);

    // Run simulation
    auto done = sim::simulate(top, instruction_mem, data_mem, 100);

    // Optionally, print data memory content
    data_mem.print_memory(0, 10);

    return 0;
}
