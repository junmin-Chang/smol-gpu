import cocotb
from cocotb.triggers import RisingEdge, FallingEdge, Timer
from cocotb.clock import Clock
import random

# Helper function to initialize signals
async def reset_dut(dut):
    dut.reset.value = 1
    dut.start.value = 0
    dut.kernel_config_reg.num_blocks.value = 0
    for i in range(len(dut.core_done)):
        dut.core_done[i].value = 0
        dut.core_start[i].value = 0
        dut.core_reset[i].value = 0
        dut.core_block_id[i].value = 0
    await Timer(10, units="ns")
    dut.reset.value = 0
    await RisingEdge(dut.clk)

@cocotb.test()
async def test_dispatcher(dut):
    # Generate a clock
    cocotb.start_soon(Clock(dut.clk, 10, units="ns").start())

    # Reset DUT
    await reset_dut(dut)

    # Test Parameters
    NUM_CORES = len(dut.core_done)  # Assuming NUM_CORES is the same as core_done width
    total_blocks = 8               # Number of total blocks for this test
    dut.kernel_config_reg.num_blocks.value = total_blocks

    # Start Kernel Execution
    dut.start.value = 1
    await RisingEdge(dut.clk)

    # Wait for cores to be reset and dispatch the first blocks
    dispatched_blocks = 0
    while dispatched_blocks < total_blocks:
        await RisingEdge(dut.clk)

        # Dispatch blocks to cores
        for i in range(NUM_CORES):
            if dut.core_reset[i].value == 1:
                dut.core_reset[i].value = 0  # Simulate that the core reset is complete
                if dispatched_blocks < total_blocks:
                    assert dut.core_start[i].value == 1, f"Core {i} did not start as expected"
                    assert dut.core_block_id[i].value == dispatched_blocks, \
                        f"Core {i} was assigned the wrong block ID: {dut.core_block_id[i].value}"
                    dispatched_blocks += 1
                else:
                    assert dut.core_start[i].value == 0, f"Core {i} started when no blocks were left"

        # Simulate core completion
        for i in range(NUM_CORES):
            if dut.core_start[i].value == 1:
                dut.core_done[i].value = 1  # Simulate core finishing the block

        # After finishing, cores should reset
        for i in range(NUM_CORES):
            if dut.core_done[i].value == 1:
                assert dut.core_reset[i].value == 1, f"Core {i} did not reset after completing a block"
                dut.core_done[i].value = 0  # Simulate the core reset

    # Ensure the kernel execution is marked as done
    await RisingEdge(dut.clk)
    assert dut.done.value == 1, "Dispatcher did not mark the kernel execution as done"
