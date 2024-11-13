# Makefile

# defaults
SIM ?= verilator
TOPLEVEL_LANG ?= verilog

VERILOG_SOURCES += $(PWD)/src/gpu/gpu.sv
VERILOG_SOURCES += $(PWD)/src/shader_core/shader_core.sv
# use VHDL_SOURCES for VHDL files

# TOPLEVEL is the name of the toplevel module in your Verilog or VHDL file
TOPLEVEL = gpu

# MODULE is the basename of the Python test file
MODULE = test.test_gpu

# include cocotb's make rules to take care of the simulator setup
include $(shell cocotb-config --makefiles)/Makefile.sim
