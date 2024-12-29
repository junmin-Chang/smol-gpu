# RISC-V GPU
The purpose of the project was to design an open-source that is simple enough for educational purposes.
At its core, it expands upon [tiny-gpu](https://github.com/adam-maj/tiny-gpu), which is a project with a similar goal.
There are, however, several changes that bring it slightly closer to what a real GPU might look like:
- a more advanced ISA (RISC-V based),
- Multiple warps per core (SIMT),
- Branching support

## ISA
The GPU itself, is based on a 32-bit word, 32-bit address space ISA that closely resembles RV32I.
Some of the instructions that don't apply to a GPU design have been cut out (fence, csrrw, etc).
Also, currently, there is also no support for unsigned arithmetic instructions.

The biggest differences were made in order to support branching.
Because the warps execute in lockstep (multiple threads executing the same instruction), it gets complicated when different threads within as a single warp diverge.

In order to solve this problem, each of the warps has its own set of registers and can execute the same instructions as each of the threads.
One of these warp registers (`s1`) is an execution mask.
Each of the bits within that register contains the information on whether a particular thread within this warp should execute the current instruction.

In order to differentiate between the warp and thread registers or instructions, the first ones will be called **scalar** and the second ones will be called **vector**.

### Vector Registers
Each of the threads within a warp has 32 of 32-bit registers.
As mentioned above, those are called vector registers and will be denoted with an `x` prefix (`x0`-`x31`).

Just like RV32I, `x0` is a read-only register with value 0.
However, for the purposes of GPU programming, registers `x1` - `x3` are also read-only and have a special purpose.
Namely, they contain the thread id, block id and block size, in that order.

The rest of the registers (`x4` - `x31`) are general purpose.

|**Register**|**Function**   |
|------------|---------------|
|`x0`        |zero           |
|`x1`        |thread id      |
|`x2`        |block id       |
|`x3`        |block size     |
|`x4`-`x31`  |general purpose|

### Scalar registers
Similarly to their vector counter part, there are 32 scalar registers that hold 32-bit words.
In order to differentiate between them, the scalar registers are prefixed with `s` (`s0`-`s31`).
The zero-th register is also tied to 0.

Register `s1` is called the execution mask and has a special purpose but is not read-only.
Each of the bits in that register denotes whether the corresponding thread should execute the current instruction.

This is also the reason why the GPU can be configured to have at most 32 threads per warp (size of the register).


|**Register**|**Function**   |
|------------|---------------|
|`s0`        |zero           |
|`s1`        |execution mask |
|`s2`-`x31`  |general purpose|

### Instructions
The instructions are split into three types:
- vector instructions
- scalar instructions
- vector-scalar instructions

Vector instructions are executed by each thread on the vector registers, scalar instructions by each warp on the scalar registers and the vector-scalar instructions are a mix (more on that later).

Which instruction is being executed is determined by three values:
- opcode,
- funct3,
- funct7

All of the vector instructions have their scalar equivalent but not vice versa.
Specifically, the jump and branch instructions are scalar-only (`jal`, `jalr`, `beq`, `bne`, `blt`, `bge`).

The most significant bit of the opcode is always equal to 0 for vector instruction and to 1 for other types.
That means, that changing the instruction type from vector to scalar is equivalent to this operation `(opcode) & (1 << 6)`.

#### Instruction list
Below is the instruction list.
The `S` bit in opcode denotes whether the instruction is vector or scalar with (1 - scalar, 0 - vector).
| mnemonic | opcode  | funct3 | funct7    |
|----------|---------|--------|-----------|
| **U-type**    |        |          |     |
| lui      | S110111 |   —    |     —     |
| auipc    | S010111 |   —    |     —     |
| **I-type arithmetic**  |          |     |
| addi     | S010011 | 000    |     —     |
| slti     | S010011 | 010    |     —     |
| xori     | S010011 | 100    |     —     |
| ori      | S010011 | 110    |     —     |
| andi     | S010011 | 111    |     —     |
| slli     | S010011 | 001    | 0000000X  |
| srli     | S010011 | 101    | 0000000X  |
| srai     | S010011 | 101    | 0100000X  |
| **R-type**    |        |          |     |
| add      | S110011 | 000    | 00000000  |
| sub      | S110011 | 000    | 01000000  |
| sll      | S110011 | 001    | 00000000  |
| slt      | S110011 | 010    | 00000000  |
| xor      | S110011 | 100    | 00000000  |
| srl      | S110011 | 101    | 00000000  |
| sra      | S110011 | 101    | 01000000  |
| or       | S110011 | 110    | 00000000  |
| and      | S110011 | 111    | 00000000  |
| **Load**      |        |          |     |
| lb       | S000011 | 000    |     —     |
| lh       | S000011 | 001    |     —     |
| lw       | S000011 | 010    |     —     |
| **Store**     |        |          |     |
| sb       | S100011 | 000    |     —     |
| sh       | S100011 | 001    |     —     |
| sw       | S100011 | 010    |     —     |
| **J-type**    |        |          |     |
| jal      | 1110111 |  —     |     —     |
| **I-type jumps** |     |          |     |
| jalr     | 1110011 | 000    |     —     |
| **B-type**    |        |          |     |
| beq      | 1110011 | 000    |     —     |
| bne      | 1110011 | 001    |     —     |
| blt      | 1110011 | 100    |     —     |
| bge      | 1110011 | 101    |     —     |
| **HALT**      |        |          |     |
| halt     | 1111111 |   —    |     —     |
| **SX type**   |        |          |     |
| sx.slt   | 1111110 |   —    |     —     |
| sx.slti  | 1111101 |   —    |     —     |

## Assembly
Currently, the supported assembly is quite simple.
Most importantly, it doesn't support multiple files or importing (you can only pass a single file with the kernel to the simulator).

There are two directives supported:
- `.blocks <num_blocks>` - denotes the number of blocks to dispatch to the GPU,
- `.warps <num_blocks>` - denotes the number of warps to execute per each block

Together they form an API similar to that of CUDA:
```cuda
kernel<<<numBlocks, threadsPerBlock>>>(args...)`
```
The key difference being that CUDA allows you to set the number of threads per block while this GPU accepts the number of warps per block as a kernel parameter. 
A compiler developer can still implement the CUDA API using execution masking.

### Syntax
The general syntax looks as follows:
```
<mnemonic> <dest>, <src1>, <src2>     ; For R-type
<mnemonic> <dest>, <src1>, <imm>      ; For I-type
<mnemonic> <dest>, <imm>              ; For U-type
<mnemonic> <dest>, <offset>(<base>)   ; For Load/Store
<mnemonic>                            ; For HALT
<mnemonic> <dest>, <src1>, <src2>     ; For SX-type (vector-scalar) (dest has to be scalar)
```
In order to turn the instruction from vector to scalar you can add the `s.` prefix.
So if you want to execute the scalar version of `addi` you would put `s.addi` as the mnemonic and use scalar registers as `src` and `dest`.

Each of the operands must be separated by a comma.

The comments are single line and the comment char is `#`.

#### Example
An example program might look like this:
```python
.blocks 32
.warps 12

# This is a comment
addi x5, x1, 1      # x5 := thread_id + 1
sx.slti s1, x5, 5   # s1[thread_id] := x5 < 5
sw x5, 0(x1)        # mem[thread_id] := x5
halt                # Stop the execution
```


## Project structure
The project is split into several subdirectories, namely:
- `external` - contains external dependencies (e.g. doctest)
- `src` - contains the system-verilog implementation of the GPU
- `sim` - contains the verilator based simulation environment and the assembler
- `test` - contains test files for the GPU, the assembler and the simulator

## Simulation
The prerequistes for running the simulation are:
- [verilator](https://www.veripool.org/wiki/verilator)
- [cmake](https://cmake.org/)
- A C++ compiler that supports C++23 (e.g. g++-14)

Verilator is a tool that can simulate or compile system-verilog code.
In this project, verilator translates the system-verilog code into C++ which then gets included as a library in the simulator.
Once the prerequistes are installed, you can build and run the simulator executable or the tests.
There are currently two ways to do this:

### Justfile
First, and the more convenient way, is to use the provided [justfile](https://github.com/casey/just).
`Just` is a modern alternative to `make`, which makes it slightly more sane to write build scripts with.
In the case of this project, the justfile is a very thin wrapper around cmake.
The available recipes are as follows:
- `compile` - builds the verilated GPU and the simulator
- `run <input_file.as> <data_file.bin>` - builds and then runs the simulator with the given assembly file
- `test` - runs the tests for the GPU, the assembler and the simulator
- `clean` - removes the build directory

In order to use it, just type `just <recipe>` in one of the subdirectories.

**Note, that the paths you pass as arguments to the `run` recipe are relative to the root of the project.
This is due to the way that the `just` command runner works.**

### CMake
As mentioned, the justfile is only a wrapper around cmake.
In case you want to use it directly, follow these steps:
```bash
mkdir build
cd build
cmake ..
cmake --build . -j$(nproc)
# The executable is build/sim/simulator
# You can also run the tests with the ctest command when in the build directory
```

### Running the simulator
The produced exectuable is located at `build/sim/simulator` (or you can just use the justfile).
You can run it in the following way:
```bash
./build/sim/simulator <input_file.as> <data_file.bin>
```
The simulator will first assemble the input file and load the binary data file into the GPU data memory.
The program will fail if the assembly code contained in the input file is ill-formed.

In case it manages to assemble the code, it will then run the simulation and print the first 100 words of the memory to the console.
This is a temporary solution and will be replaced by a more sophisticated output mechanism in the future.