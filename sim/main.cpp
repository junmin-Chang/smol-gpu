#include <print>
#include "sim.hpp"

auto main() -> int {
    std::println("Hello from cpp!");

    Vgpu top{};

    constexpr auto num_channels = 8;
    constexpr auto mem_cells_count = 2048;
    auto data_mem = sim::make_data_memory<mem_cells_count, num_channels>(&top);
    auto instruction_mem = sim::make_instruction_memory<mem_cells_count, num_channels>(&top);


    // addi 5 to x5
    IData opcode = 0b0010011;
    IData rd = 5;
    IData rs1 = 1;
    IData imm = 5;
    IData instruction = opcode | (rd << 7) | (rs1 << 15) | (imm << 20);

    // store x5 at x1
    const IData opcode2 = 0b0100011;
    const IData offset = 0;
    const IData funct3 = 0b010;
    const IData rs2 = 5;
    rs1 = 1;
    const IData instruction2 = opcode2 | (offset << 7) | (funct3 << 12) | (rs1 << 15) | (rs2 << 20);


    const IData finish_instruction = 0xffffffff;
    instruction_mem.load_instruction(0, instruction);
    instruction_mem.load_instruction(1, instruction2);
    instruction_mem.load_instruction(2, finish_instruction);

    // Prepare kernel configuration
    sim::set_kernel_config(top, 0, 0, 1, 1);

    // Start the execution
    top.execution_start = 1;

    // Simulate for a certain number of cycles
    for (int cycle = 0; cycle < 500; ++cycle) {
        // Evaluate the DUT
        top.eval();

        if(top.execution_done) {
            std::println("Finishing simulation successfully.");
            break;
        }

        // Process instruction memory reads
        instruction_mem.process();

        // Process data memory reads and writes
        data_mem.process();

        top.eval();

        // Toggle the clock
        sim::tick(top);
    }

    // Optionally, print data memory content
    data_mem.print_memory(0, 10);

    return 0;
}
