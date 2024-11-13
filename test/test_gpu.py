import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge

@cocotb.test()
async def test_gpu_top(dut):
    """ Test GPU top module output """

    # Clock setup
    for _ in range(10):  # Run for a few cycles
        await RisingEdge(dut.clk)
        assert dut.test_out.value == 1, "test_out should be high (1'b1)"
