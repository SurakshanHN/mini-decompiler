#pragma once
#include <cstdint>
#include <vector>
#include <string>

struct TextSection {
    std::vector<uint8_t> bytes;   
    uint64_t load_address;        
    uint64_t file_offset;         
};

TextSection parse_elf(const std::string& path);
