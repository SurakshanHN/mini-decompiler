#include "elf_parser.h"
#include <elf.h>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <cstring>

TextSection parse_elf(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + path);
    }

    Elf64_Ehdr ehdr;
    if (!file.read(reinterpret_cast<char*>(&ehdr), sizeof(ehdr))) {
        throw std::runtime_error("Could not read ELF header");
    }

    if (std::memcmp(ehdr.e_ident, ELFMAG, SELFMAG) != 0) {
        throw std::runtime_error("Not a valid ELF file");
    }

    if (ehdr.e_ident[EI_CLASS] != ELFCLASS64) {
        throw std::runtime_error("Only 64-bit ELFs are supported");
    }

    file.seekg(ehdr.e_shoff, std::ios::beg);
    std::vector<Elf64_Shdr> shdrs(ehdr.e_shnum);
    if (!file.read(reinterpret_cast<char*>(shdrs.data()), ehdr.e_shnum * sizeof(Elf64_Shdr))) {
        throw std::runtime_error("Could not read section headers");
    }

    const Elf64_Shdr& shstrtab_sh = shdrs[ehdr.e_shstrndx];
    std::vector<char> shstrtab(shstrtab_sh.sh_size);
    file.seekg(shstrtab_sh.sh_offset, std::ios::beg);
    if (!file.read(shstrtab.data(), shstrtab_sh.sh_size)) {
        throw std::runtime_error("Could not read section name string table");
    }
    for (const auto& sh : shdrs) {
        std::string name = &shstrtab[sh.sh_name];
        if (name == ".text") {
            TextSection ts;
            ts.load_address = sh.sh_addr;
            ts.file_offset = sh.sh_offset;
            ts.bytes.resize(sh.sh_size);
            
            file.seekg(sh.sh_offset, std::ios::beg);
            if (!file.read(reinterpret_cast<char*>(ts.bytes.data()), sh.sh_size)) {
                throw std::runtime_error("Could not read .text section bytes");
            }
            return ts;
        }
    }

    throw std::runtime_error(".text section not found");
}
