source_files := `find ~+/src -type f -name "*.sv" | xargs echo`
output_dir := "build"
test_output_dir := "test_build"
output_exe := "gpu"
common_dir := `echo $(pwd)/src/common`
num_cores := `nproc`
cocotb_makefiles := `cocotb-config --makefiles`

CC := "g++"

default: run

compile:
    mkdir -p {{output_dir}}
    cd {{output_dir}} && cmake .. && cmake --build . -j{{num_cores}}

run: compile
    ./{{output_dir}}/sim/simulator

test name:
    export SIM="verilator" ; \
    export TOPLEVEL_LANG="verilog" ; \
    export MODULE="test.test_{{name}}" ; \
    export TOPLEVEL="{{name}}" ; \
    export MAKEFILE="{{cocotb_makefiles}}/Makefile.sim" ; \
    export VERILOG_SOURCES="{{source_files}}" ; \
    export EXTRA_ARGS="-j {{num_cores}} -I{{common_dir}} -CFLAGS -std=c++20" ; \
    export SIM_BUILD="{{test_output_dir}}" ; \
    make -f "$MAKEFILE"

clean:
    rm -rf {{output_dir}}
    rm -f results.xml
