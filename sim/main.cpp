#include <print>
#include "sim.hpp"

auto main() -> int {
    std::println("Hello from cpp!");

    using namespace sim::instructions;

    Vgpu top{};

    constexpr auto num_channels = 8;
    constexpr auto mem_cells_count = 2048;
    auto data_mem = sim::make_data_memory<mem_cells_count, num_channels>(&top);
    auto instruction_mem = sim::make_instruction_memory<mem_cells_count, num_channels>(&top);

    data_mem.push_data(IData{1} << 2);

    auto mask_instruction = lw(1, 0, 0).make_scalar();
    auto jal_instruction = jal(8, 10);

    instruction_mem.push_instruction(addi(5, 1 ,0));
    instruction_mem.push_instruction(sx_slti(1, 5, 5));
    instruction_mem.push_instruction(sw(5, 1, 0));
    instruction_mem.push_instruction(halt());

    // Prepare kernel configuration
    sim::set_kernel_config(top, 0, 0, 1, 1);

    // Run simulation
    auto done = sim::simulate(top, instruction_mem, data_mem, 200);

    if(!done) {
        std::println("Simulation failed!");
        return 1;
    }

    // Optionally, print data memory content
    data_mem.print_memory(0, 10);

    return 0;
}
