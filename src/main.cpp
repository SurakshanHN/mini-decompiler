#include <iostream>
#include <iomanip>
#include "elf_parser.h"
#include "disassembler.h"
#include "lifter.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <elf-binary> [<hex-addr>]\n";
        return 1;
    }
    try {
        TextSection ts = parse_elf(argv[1]);

        uint64_t func_addr = 0x1119; 
        if (argc >= 3) {
            func_addr = std::stoull(argv[2], nullptr, 16);
        }

        size_t func_offset = func_addr - ts.load_address;
        
        if (func_offset >= ts.bytes.size()) {
            std::cerr << "Error: function address 0x" << std::hex << func_addr << " out of range\n";
            return 1;
        }

        const uint8_t* func_ptr  = ts.bytes.data() + func_offset;
        size_t         func_size = ts.bytes.size() - func_offset;

        auto instructions = disassemble(func_ptr, func_size, func_addr);

        std::cout << "[Disassembly]\n";
        for (const auto& ins : instructions) {
            std::cout << "  0x" << std::hex << std::setw(4)
                      << std::setfill('0') << ins.address;
            std::cout << "  " << std::left << std::setw(8) << std::setfill(' ')
                      << ins.mnemonic << " " << ins.op_str << "\n";
        }

        std::cout << "\n[Pseudo-C Output]\n";
        std::string pseudo_c = lift(instructions);
        std::cout << pseudo_c << "\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
