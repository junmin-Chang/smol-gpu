source_files := `find ~+/src -type f -name "*.sv" | xargs echo`
output_dir := "build"
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

test: compile
    cd {{output_dir}} && ctest -j{{num_cores}} --output-on-failure
 
clean:
    rm -rf {{output_dir}}
    rm -f results.xml
