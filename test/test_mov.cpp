#include "Vgpu_gpu.h"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include "sim.hpp"

constexpr auto INST_NUM_CHANNELS = Vgpu_gpu::INSTRUCTION_MEM_NUM_CHANNELS;
constexpr auto DATA_NUM_CHANNELS = Vgpu_gpu::DATA_MEM_NUM_CHANNELS;

TEST_CASE("mov + sw + halt") {
    auto gpu = Vgpu{};

    auto instruction_memory = sim::make_instruction_memory<1024, INST_NUM_CHANNELS>(&gpu);
    auto data_memory = sim::make_data_memory<1024, DATA_NUM_CHANNELS>(&gpu);

    instruction_memory.push_instruction(sim::addi(5, 1, 0));
    instruction_memory.push_instruction(sim::sw(5, 1, 0));
    instruction_memory.push_instruction(sim::halt());

    sim::set_kernel_config(gpu, 0, 0, 1, 1);

    auto done = sim::simulate(gpu, instruction_memory, data_memory, 100);

    REQUIRE(done);
    for(auto i = 0; i < 32; i++) {
        CHECK(data_memory[i] == i);
    }
}

TEST_CASE("lw + sw") {
    auto gpu = Vgpu{};

    auto instruction_memory = sim::make_instruction_memory<1024, INST_NUM_CHANNELS>(&gpu);
    auto data_memory = sim::make_data_memory<1024, DATA_NUM_CHANNELS>(&gpu);

    data_memory.push_data(10);
    data_memory.push_data(20);
    data_memory.push_data(30);

    instruction_memory.push_instruction(sim::lw(6, 0, 0));
    instruction_memory.push_instruction(sim::sw(1, 6, 0));
    instruction_memory.push_instruction(sim::halt());

    sim::set_kernel_config(gpu, 0, 0, 1, 1);

    auto done = sim::simulate(gpu, instruction_memory, data_memory, 10000);

    REQUIRE(done);
    for(auto i = 0; i < 32; i++) {
        CHECK(data_memory[i] == 10);
    }
}
