source_files := `find ~+/src -type f -name "*.sv" | xargs echo`
output_dir := "build"
output_exe := "gpu"
num_cores := `nproc`
cocotb_makefiles := `cocotb-config --makefiles`

default: run

compile:
    mkdir -p {{output_dir}}
    verilator --binary -j {{num_cores}} -CFLAGS "-std=c++20" -Mdir {{output_dir}} {{source_files}} -o {{output_exe}}

run: compile
    {{output_dir}}/{{output_exe}}

test:
    export SIM="verilator" ; \
    export TOPLEVEL_LANG="verilog" ; \
    export MODULE="test.test_gpu" ; \
    export TOPLEVEL="gpu" ; \
    export MAKEFILE="{{cocotb_makefiles}}/Makefile.sim" ; \
    export VERILOG_SOURCES="{{source_files}}" ; \
    export EXTRA_ARGS="-j {{num_cores}} -CFLAGS -std=c++20" ; \
    export SIM_BUILD="{{output_dir}}" ; \
    make -f "$MAKEFILE"

clean:
    rm -rf {{output_dir}}
    rm -f results.xml
