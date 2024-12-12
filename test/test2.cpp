#include "Vgpu_gpu.h"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include "sim.hpp"

constexpr auto INST_NUM_CHANNELS = Vgpu_gpu::INSTRUCTION_MEM_NUM_CHANNELS;
constexpr auto DATA_NUM_CHANNELS = Vgpu_gpu::DATA_MEM_NUM_CHANNELS;

// ALU operation tests
TEST_CASE("ALU operations") {
    SUBCASE("sub") {
        auto top = Vgpu{};

        auto data_mem = sim::make_data_memory<2048, 8>(&top);
        auto instruction_mem = sim::make_instruction_memory<2048, 8>(&top);

        data_mem.push_data(50); // data_mem[0] = 50
        data_mem.push_data(20); // data_mem[1] = 20

        instruction_mem.push_instruction(sim::lw(6, 0, 0)); // lw x6, 0(x0)
        instruction_mem.push_instruction(sim::lw(5, 0, 1)); // lw x5, 1(x0)
        instruction_mem.push_instruction(sim::sub(7, 6, 5)); // sub x7, x6, x5
        instruction_mem.push_instruction(sim::sw(1, 7, 0));  // sw x7, 0(x1)
        instruction_mem.push_instruction(sim::halt());

        // Prepare kernel configuration
        sim::set_kernel_config(top, 0, 0, 1, 1);

        // Run simulation
        auto done = sim::simulate(top, instruction_mem, data_mem, 2000);

        REQUIRE(done);
        for(auto i = 0; i < 32; i++) {
            CHECK(data_mem[i] == 30);
        }
    }

    SUBCASE("and") {
        auto top = Vgpu{};

        auto data_mem = sim::make_data_memory<2048, 8>(&top);
        auto instruction_mem = sim::make_instruction_memory<2048, 8>(&top);

        data_mem.push_data(0b1100); // data_mem[0] = 12
        data_mem.push_data(0b1010); // data_mem[1] = 10

        instruction_mem.push_instruction(sim::lw(6, 0, 0)); // lw x6, 0(x0)
        instruction_mem.push_instruction(sim::lw(5, 0, 1)); // lw x5, 1(x0)
        instruction_mem.push_instruction(sim::and_(7, 6, 5)); // and x7, x6, x5
        instruction_mem.push_instruction(sim::sw(1, 7, 0));  // sw x7, 0(x1)
        instruction_mem.push_instruction(sim::halt());

        // Prepare kernel configuration
        sim::set_kernel_config(top, 0, 0, 1, 1);

        // Run simulation
        auto done = sim::simulate(top, instruction_mem, data_mem, 2000);

        REQUIRE(done);
        for(auto i = 0; i < 32; i++) {
            CHECK(data_mem[i] == 8);
        }
    }

    SUBCASE("or") {
        auto top = Vgpu{};

        auto data_mem = sim::make_data_memory<2048, 8>(&top);
        auto instruction_mem = sim::make_instruction_memory<2048, 8>(&top);

        data_mem.push_data(0b1100); // data_mem[0] = 12
        data_mem.push_data(0b1010); // data_mem[1] = 10

        instruction_mem.push_instruction(sim::lw(6, 0, 0)); // lw x6, 0(x0)
        instruction_mem.push_instruction(sim::lw(5, 0, 1)); // lw x5, 1(x0)
        instruction_mem.push_instruction(sim::or_(7, 6, 5)); // or x7, x6, x5
        instruction_mem.push_instruction(sim::sw(1, 7, 0));  // sw x7, 0(x1)
        instruction_mem.push_instruction(sim::halt());

        // Prepare kernel configuration
        sim::set_kernel_config(top, 0, 0, 1, 1);

        // Run simulation
        auto done = sim::simulate(top, instruction_mem, data_mem, 2000);

        REQUIRE(done);
        for(auto i = 0; i < 32; i++) {
            CHECK(data_mem[i] == 14);
        }
    }

    SUBCASE("xor") {
        auto top = Vgpu{};

        auto data_mem = sim::make_data_memory<2048, 8>(&top);
        auto instruction_mem = sim::make_instruction_memory<2048, 8>(&top);

        data_mem.push_data(0b1100); // data_mem[0] = 12
        data_mem.push_data(0b1010); // data_mem[1] = 10

        instruction_mem.push_instruction(sim::lw(6, 0, 0)); // lw x6, 0(x0)
        instruction_mem.push_instruction(sim::lw(5, 0, 1)); // lw x5, 1(x0)
        instruction_mem.push_instruction(sim::xor_(7, 6, 5)); // xor x7, x6, x5
        instruction_mem.push_instruction(sim::sw(1, 7, 0));   // sw x7, 0(x1)
        instruction_mem.push_instruction(sim::halt());

        // Prepare kernel configuration
        sim::set_kernel_config(top, 0, 0, 1, 1);

        // Run simulation
        auto done = sim::simulate(top, instruction_mem, data_mem, 2000);

        REQUIRE(done);
        for(auto i = 0; i < 32; i++) {
            CHECK(data_mem[i] == 6);
        }
    }

    SUBCASE("shift left logical") {
        auto top = Vgpu{};

        auto data_mem = sim::make_data_memory<2048, 8>(&top);
        auto instruction_mem = sim::make_instruction_memory<2048, 8>(&top);

        data_mem.push_data(1); // data_mem[0] = 1
        data_mem.push_data(3); // data_mem[1] = 3

        instruction_mem.push_instruction(sim::lw(6, 0, 0)); // lw x6, 0(x0)
        instruction_mem.push_instruction(sim::lw(5, 0, 1)); // lw x5, 1(x0)
        instruction_mem.push_instruction(sim::sll(7, 6, 5)); // sll x7, x6, x5
        instruction_mem.push_instruction(sim::sw(1, 7, 0));  // sw x7, 0(x1)
        instruction_mem.push_instruction(sim::halt());

        // Prepare kernel configuration
        sim::set_kernel_config(top, 0, 0, 1, 1);

        // Run simulation
        auto done = sim::simulate(top, instruction_mem, data_mem, 2000);

        REQUIRE(done);
        for(auto i = 0; i < 32; i++) {
            CHECK(data_mem[i] == 8);
        }
    }

    SUBCASE("shift right logical") {
        auto top = Vgpu{};

        auto data_mem = sim::make_data_memory<2048, 8>(&top);
        auto instruction_mem = sim::make_instruction_memory<2048, 8>(&top);

        data_mem.push_data(8); // data_mem[0] = 8
        data_mem.push_data(3); // data_mem[1] = 3

        instruction_mem.push_instruction(sim::lw(6, 0, 0)); // lw x6, 0(x0)
        instruction_mem.push_instruction(sim::lw(5, 0, 1)); // lw x5, 1(x0)
        instruction_mem.push_instruction(sim::srl(7, 6, 5)); // srl x7, x6, x5
        instruction_mem.push_instruction(sim::sw(1, 7, 0));  // sw x7, 0(x1)
        instruction_mem.push_instruction(sim::halt());

        // Prepare kernel configuration
        sim::set_kernel_config(top, 0, 0, 1, 1);

        // Run simulation
        auto done = sim::simulate(top, instruction_mem, data_mem, 2000);

        REQUIRE(done);
        for(auto i = 0; i < 32; i++) {
            CHECK(data_mem[i] == 1);
        }
    }

    SUBCASE("addi") {
        auto top = Vgpu{};

        auto data_mem = sim::make_data_memory<2048, 8>(&top);
        auto instruction_mem = sim::make_instruction_memory<2048, 8>(&top);

        instruction_mem.push_instruction(sim::addi(6, 1, 10)); // addi x6, x1, 10
        instruction_mem.push_instruction(sim::sw(1, 6, 0));    // sw x6, 0(x1)
        instruction_mem.push_instruction(sim::halt());

        sim::set_kernel_config(top, 0, 0, 1, 1);

        auto done = sim::simulate(top, instruction_mem, data_mem, 2000);
        REQUIRE(done);

        // data_mem[i] = i + 10
        for(auto i = 0; i < 32; i++) {
            CHECK(data_mem[i] == i + 10);
        }
    }

}

TEST_CASE("Multiple warps") {
    auto top = Vgpu{};

    auto data_mem = sim::make_data_memory<2048, DATA_NUM_CHANNELS>(&top);
    auto instruction_mem = sim::make_instruction_memory<2048, INST_NUM_CHANNELS>(&top);

    instruction_mem.push_instruction(sim::addi(5, 1, 0)); // x5 = x1
    instruction_mem.push_instruction(sim::sw(1, 5, 0));   // sw x5, 0(x1)
    instruction_mem.push_instruction(sim::halt());

    sim::set_kernel_config(top, 0, 0, 1, 2); // 1 block, 2 warps per block

    auto done = sim::simulate(top, instruction_mem, data_mem, 5000);
    REQUIRE(done);

    for(auto i = 0; i < 64; i++) {
        CHECK(data_mem[i] == i);
    }

}

TEST_CASE("Multiple blocks") {
    auto top = Vgpu{};

    auto data_mem = sim::make_data_memory<4096, DATA_NUM_CHANNELS>(&top);
    auto instruction_mem = sim::make_instruction_memory<4096, INST_NUM_CHANNELS>(&top);

    instruction_mem.push_instruction(sim::addi(5, 2, 0)); // x5 = x2 (block id)
    instruction_mem.push_instruction(sim::sw(1, 5, 0));   // sw x5, 0(x1)
    instruction_mem.push_instruction(sim::halt());

    sim::set_kernel_config(top, 0, 0, 2, 1); // 2 blocks, 1 warp per block

    auto done = sim::simulate(top, instruction_mem, data_mem, 5000);
    REQUIRE(done);

    for(auto i = 0; i < 64; i++) {
        if (i < 32)
            CHECK(data_mem[i] == 1);
    }

}

TEST_CASE("Conditional execution using mask") {
    auto top = Vgpu{};

    auto data_mem = sim::make_data_memory<2048, DATA_NUM_CHANNELS>(&top);
    auto instruction_mem = sim::make_instruction_memory<2048, INST_NUM_CHANNELS>(&top);

    instruction_mem.push_instruction(sim::sx_slti(1, 1, 3));    // s1 = (x1 < 3) ? 1 : 0
    instruction_mem.push_instruction(sim::addi(5, 0, 10));      // x5 = 10
    instruction_mem.push_instruction(sim::sw(1, 5, 0));         // sw x5, 0(x1)
    instruction_mem.push_instruction(sim::halt());

    sim::set_kernel_config(top, 0, 0, 1, 1);

    auto done = sim::simulate(top, instruction_mem, data_mem, 5000);
    REQUIRE(done);

    for(auto i = 0; i < 32; i++) {
        if (i < 3)
            CHECK(data_mem[i] == 10);
        else
            CHECK(data_mem[i] == 0);
    }

}
