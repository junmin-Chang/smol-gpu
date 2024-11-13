source_files := `find src -type f -name "*.sv" | xargs echo`
output_dir := "build"
output_exe := "gpu"
num_cores := `nproc`

default: run

compile:
    mkdir -p {{output_dir}}
    verilator --binary -j {{num_cores}} -CFLAGS "-std=c++20" -Mdir {{output_dir}} {{source_files}} -o {{output_exe}}

run: compile
    {{output_dir}}/{{output_exe}}

test:
    SIM=verilator MODULE=test.test_gpu make

clean:
    rm -rf {{output_dir}}/*
