#pragma once
#include <print>
#include <array>
#include <bitset>
#include "Vgpu.h"

namespace sim {

struct Instruction {
    IData bits{};

    Instruction() = default;
    Instruction(IData bits) : bits(bits) {}

    operator IData() {
        return bits;
    }

    constexpr auto set_opcode(IData opcode) -> Instruction& {
        bits |= opcode;
        return *this;
    }

    constexpr auto set_rd(IData rd) -> Instruction& {
        bits |= rd << 7;
        return *this;
    }

    constexpr auto set_funct3(IData funct3) -> Instruction& {
        bits |= funct3 << 12;
        return *this;
    }

    constexpr auto set_rs1(IData rs1) -> Instruction& {
        bits |= rs1 << 15;
        return *this;
    }

    constexpr auto set_imm(IData imm) -> Instruction& {
        bits |= imm << 20;
        return *this;
    }

    constexpr auto set_rs2(IData rs2) -> Instruction& {
        bits |= rs2 << 20;
        return *this;
    }

    constexpr auto set_funct7(IData funct7) -> Instruction& {
        bits |= funct7 << 25;
        return *this;
    }

    constexpr auto set_imm20(IData imm20) -> Instruction& {
        bits |= imm20 << 12;
        return *this;
    }

    constexpr auto set_imm21(IData imm21) -> Instruction& {
        auto imm_j = std::bitset<32>{};
        auto imm = std::bitset<21>(imm21);

        imm_j[31] = imm[20];
        for(auto i = 21u; i <= 30u; i++) {
            imm_j[i] = imm[i - 20];
        }
        imm_j[20] = imm[11];
        for(auto i = 12u; i <= 19u; i++) {
            imm_j[i] = imm[i];
        }
        imm[0] = 0;

        bits |= imm_j.to_ulong();
        return *this;
    }

    constexpr auto set_imm12(IData imm12) -> Instruction& {
        bits |= imm12 << 20;
        return *this;
    }

    constexpr auto make_scalar() -> Instruction& {
        bits |= 1 << 6;
        return *this;
    }

    constexpr auto make_vector() -> Instruction& {
        bits &= ~(1 << 6);
        return *this;
    }
};

// opcodes
constexpr IData OPCODE_R        = 0b0110011;         // Used by all R-type instructions (ADD, SUB, SLL, SLT, XOR, SRL, SRA)
constexpr IData OPCODE_I        = 0b0010011;         // Used by ALU I-type instructions (ADDI, SLTI, XORI, ORI, ANDI, SLLI, SRLI, SRAI)
constexpr IData OPCODE_S        = 0b0100011;         // Used by store instructions (SB, SH, SW)
constexpr IData OPCODE_B        = 0b1100011;         // Used by branch instructions (BEQ, BNE, BLT, BGE)
constexpr IData OPCODE_U        = 0b0110111;         // Used by LUI
constexpr IData OPCODE_I_LOAD   = 0b0000011;    // Used by load instructions (LB, LH, LW)
constexpr IData OPCODE_AUIPC    = 0b0010111;     // Used by AUIPC
constexpr IData OPCODE_HALT     = 0b1111111;     // Used by HALT

// funct3
constexpr IData ADDI            = 0b000;            // ADDI
constexpr IData SLTI            = 0b010;            // SLTI
constexpr IData ANDI            = 0b111;            // ANDI
constexpr IData ORI             = 0b110;            // ORI
constexpr IData XORI            = 0b100;            // XORI

constexpr IData LW              = 0b010;            // LW
constexpr IData LH              = 0b001;            // LH
constexpr IData LB              = 0b000;            // LB

constexpr auto halt() -> IData {
    return OPCODE_HALT;
}

// I-type instruction creation
constexpr auto create_itype_instruction(IData opcode, IData funct3, IData rd, IData rs1, IData imm) -> Instruction {
    return Instruction().set_opcode(opcode).set_funct3(funct3).set_rd(rd).set_rs1(rs1).set_imm(imm);
}
constexpr auto addi(IData rd, IData rs1, IData imm) -> Instruction {
    return create_itype_instruction(OPCODE_I, ADDI, rd, rs1, imm);
}
constexpr auto slti(IData rd, IData rs1, IData imm) -> Instruction {
    return create_itype_instruction(OPCODE_I, SLTI, rd, rs1, imm);
}
constexpr auto andi(IData rd, IData rs1, IData imm) -> Instruction {
    return create_itype_instruction(OPCODE_I, ANDI, rd, rs1, imm);
}
constexpr auto ori(IData rd, IData rs1, IData imm) -> Instruction {
    return create_itype_instruction(OPCODE_I, ORI, rd, rs1, imm);
}
constexpr auto xori(IData rd, IData rs1, IData imm) -> Instruction {
    return create_itype_instruction(OPCODE_I, XORI, rd, rs1, imm);
}
constexpr auto lw(IData rd, IData rs1, IData imm) -> Instruction {
    return create_itype_instruction(OPCODE_I_LOAD, LW, rd, rs1, imm);
}

// S-type instruction creation
constexpr auto sw(IData rs1, IData rs2, IData imm) -> Instruction {
    return Instruction{}.set_opcode(OPCODE_S).set_funct3(0).set_rs1(rs1).set_rs2(rs2).set_imm12(imm);
}

// R-type instruction creation
constexpr auto create_rtype_instruction(IData opcode, IData funct3, IData rd, IData rs1, IData rs2) -> Instruction {
    return Instruction().set_opcode(opcode).set_funct3(funct3).set_rd(rd).set_rs1(rs1).set_rs2(rs2);
}
constexpr auto add(IData rd, IData rs1, IData rs2) -> Instruction {
    return create_rtype_instruction(OPCODE_R, 0, rd, rs1, rs2);
}

constexpr void tick(Vgpu& top) {
    top.clk = 0;
    top.eval();
    top.clk = 1;
    top.eval();
}

// J-type instruction creation
constexpr auto create_jtype_instruction(IData opcode, IData rd, IData imm21) -> Instruction {
    return Instruction().set_opcode(opcode).set_rd(rd).set_imm21(imm21);
}
constexpr auto jal(IData rd, IData imm20) -> Instruction {
    return create_jtype_instruction(0b1101111, rd, imm20);
}

// CData is uint8_t, IData is uint32_t
constexpr void set_bit(CData& signal, int bit, bool value) {
    if (value) {
        signal |= 1 << bit;
    } else {
        signal &= ~(1 << bit);
    }
}

constexpr bool get_bit(CData signal, int bit) {
    return (signal >> bit) & 1;
}

template <uint32_t mem_cells, uint32_t num_channels>
struct InstructionMemory {
    Vgpu* dut;
    CData *instruction_mem_read_valid;                  // input
    IData *instruction_mem_read_address[num_channels];  // input
    CData *instruction_mem_read_ready;                  // output
    IData *instruction_mem_read_data[num_channels];     // output

    std::array<IData, mem_cells> memory{};

    // Process read requests
    void process() {
        for (size_t i = 0; i < num_channels; i++) {
            if (*instruction_mem_read_valid & (1 << i)) {
                IData addr = *instruction_mem_read_address[i];
                if (addr < mem_cells) {
                    *instruction_mem_read_data[i] = memory[addr];
                } else {
                    *instruction_mem_read_data[i] = 0;
                    std::println(stderr, "Error: Read from invalid address {}", addr);
                }
                set_bit(*instruction_mem_read_ready, (int)i, true);
            } else {
                set_bit(*instruction_mem_read_ready, (int)i, false);
            }
        }
    }

    // Method to load an instruction into memory
    void load_instruction(IData addr, IData instruction) {
        if (addr < mem_cells) {
            memory[addr] = instruction;
        } else {
            std::println(stderr, "Error: Attempt to load instruction at invalid address {}", addr);
        }
    }

    void push_instruction(Instruction instruction) {
        memory[stack_ptr++] = (IData)instruction;
    }

    auto operator[](IData addr) -> IData& {
        return memory[addr];
    }

    uint32_t stack_ptr = 0u;
};

template <uint32_t mem_cells, uint32_t num_channels>
struct DataMemory {
    Vgpu* dut;
    CData *data_mem_read_valid;                  // input
    IData *data_mem_read_address[num_channels];  // input
    CData *data_mem_read_ready;                  // output
    IData *data_mem_read_data[num_channels];     // output
    CData *data_mem_write_valid;                 // input
    IData *data_mem_write_address[num_channels]; // input
    IData *data_mem_write_data[num_channels];    // input
    CData *data_mem_write_ready;                 // output

    std::array<IData, mem_cells> memory{};

    auto operator[](IData addr) -> IData& {
        return memory[addr];
    }

    // Process read and write requests
    void process() {
        // Process writes first
        for (size_t i = 0; i < num_channels; i++) {
            if (*data_mem_write_valid & (1 << i)) {
                IData addr = *data_mem_write_address[i];
                if (addr < mem_cells) {
                    memory[addr] = *data_mem_write_data[i];
                } else {
                    std::println(stderr, "Error: Write to invalid address {}", addr);
                }
                set_bit(*data_mem_write_ready, (int)i, true);
            } else {
                set_bit(*data_mem_write_ready, (int)i, false);
            }
        }

        // Then process reads
        for (size_t i = 0; i < num_channels; i++) {
            if (*data_mem_read_valid & (1 << i)) {
                IData addr = *data_mem_read_address[i];
                if (addr < mem_cells) {
                    *data_mem_read_data[i] = memory[addr];
                } else {
                    data_mem_read_data[i] = 0;
                    std::println(stderr, "Error: Read from invalid address {}", addr);
                }
                set_bit(*data_mem_read_ready, (int)i, true);
            } else {
                set_bit(*data_mem_read_ready, (int)i, false);
            }
        }
    }

    // Optional: Method to print memory content for debugging
    void print_memory(IData start_addr = 0, IData end_addr = mem_cells - 1) {
        for (IData addr = start_addr; addr <= end_addr && addr < mem_cells; ++addr) {
            std::println("Memory[{}]: {}", addr, memory[addr]);
        }
    }

    void push_data(IData data) {
        memory[stack_ptr++] = data;
    }

    uint32_t stack_ptr = 0u;
};

template <uint32_t mem_cells, uint32_t num_channels>
auto make_instruction_memory(Vgpu* dut) -> InstructionMemory<mem_cells, num_channels> {
    InstructionMemory<mem_cells, num_channels> mem{};
    mem.dut = dut;
    mem.instruction_mem_read_valid = &dut->instruction_mem_read_valid;
    mem.instruction_mem_read_ready = &dut->instruction_mem_read_ready;
    for (auto i = 0u; i < num_channels; i++) {
        mem.instruction_mem_read_address[i] = &dut->instruction_mem_read_address[i];
        mem.instruction_mem_read_data[i] = &dut->instruction_mem_read_data[i];
    }
    return mem;
}

template <uint32_t mem_cells, uint32_t num_channels>
auto make_data_memory(Vgpu* dut) -> DataMemory<mem_cells, num_channels> {
    DataMemory<mem_cells, num_channels> mem{};
    mem.dut = dut;
    mem.data_mem_read_valid = &dut->data_mem_read_valid;
    mem.data_mem_read_ready = &dut->data_mem_read_ready;
    mem.data_mem_write_valid = &dut->data_mem_write_valid;
    mem.data_mem_write_ready = &dut->data_mem_write_ready;
    for (auto i = 0u; i < num_channels; i++) {
        mem.data_mem_read_address[i] = &dut->data_mem_read_address[i];
        mem.data_mem_read_data[i] = &dut->data_mem_read_data[i];
        mem.data_mem_write_address[i] = &dut->data_mem_write_address[i];
        mem.data_mem_write_data[i] = &dut->data_mem_write_data[i];
    }
    return mem;
}

constexpr void set_kernel_config(Vgpu& top, IData base_instructions_address, IData base_data_address, IData num_blocks, IData num_warps_per_block) {
    VlWide<4>& kernel_config = top.kernel_config;
    kernel_config[3] = base_instructions_address;
    kernel_config[2] = base_data_address;
    kernel_config[1] = num_blocks;
    kernel_config[0] = num_warps_per_block;
}

template <uint32_t mem_cells, uint32_t num_channels>
bool simulate(Vgpu& top, InstructionMemory<mem_cells, num_channels>& instruction_mem, DataMemory<mem_cells, num_channels>& data_mem, uint32_t max_num_cycles) {
    top.execution_start = 1;

    for (auto cycle = 0u; cycle < max_num_cycles; ++cycle) {
        top.eval();

        if (top.execution_done) {
            return true;
        }

        instruction_mem.process();
        data_mem.process();

        top.eval();

        tick(top);
    }
    return false;
}

} // namespace sim

