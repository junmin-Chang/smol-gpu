#include <print>
#include "sim.hpp"

auto main() -> int {
    std::println("Hello from cpp!");

    Vgpu top{};

    constexpr auto num_channels = 8;
    constexpr auto mem_cells_count = 2048;
    auto data_mem = sim::make_data_memory<mem_cells_count, num_channels>(&top);
    auto instruction_mem = sim::make_instruction_memory<mem_cells_count, num_channels>(&top);

    data_mem.push_data(IData{1} << 2);

    auto mask_instruction = sim::lw(1, 0, 0).make_scalar();
    auto jal = sim::jal(8, 10);
    instruction_mem.push_instruction(sim::jal(8, 10));
    /*instruction_mem.push_instruction(sim::addi(5, 1 ,0));*/
    /*instruction_mem.push_instruction(sim::sw(5, 1, 0));*/
    instruction_mem.push_instruction(sim::halt());



    // Prepare kernel configuration
    sim::set_kernel_config(top, 0, 0, 1, 1);

    // Run simulation
    auto done = sim::simulate(top, instruction_mem, data_mem, 30);

    if(!done) {
        std::println("Simulation failed!");
        return 1;
    }

    // Optionally, print data memory content
    /*data_mem.print_memory(0, 100);*/

    return 0;
}
