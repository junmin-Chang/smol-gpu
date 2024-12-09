find_package(verilator HINTS $ENV{VERILATOR_ROOT})
if (NOT verilator_FOUND)
    message(FATAL_ERROR "Verilator not found")
endif()

set(MODULE_VERILOG_SOURCES alu.sv compute_core.sv decoder.sv dispatcher.sv fetcher.sv gpu.sv lsu.sv mem_controller.sv reg_file.sv common/common.sv)

add_library(GPU SHARED)

verilate(GPU SOURCES ${MODULE_VERILOG_SOURCES} PREFIX Vgpu TOP_MODULE gpu VERILATOR_ARGS -cc -I${CMAKE_CURRENT_SOURCE_DIR}/common -CFLAGS "-std=c++20")
