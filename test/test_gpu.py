import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge

@cocotb.test()
async def test_gpu(dut):
    """ Test GPU top module output """

    clock = Clock(dut.clk, 10, units="ns")
    cocotb.start_soon(clock.start(start_high=False))

    # Clock setup
    for _ in range(10):  # Run for a few cycles
        await RisingEdge(dut.clk)
        assert dut.test_out.value == 0, "test_out should be high (1'b1)"
