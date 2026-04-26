#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct Instruction {
    uint64_t    address;   
    std::string mnemonic;  
    std::string op_str;    
    std::vector<uint8_t> bytes; 
};

std::vector<Instruction> disassemble(
    const uint8_t* data,
    size_t         size,
    uint64_t       base_address
);
