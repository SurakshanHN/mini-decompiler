#include "disassembler.h"
#include <capstone/capstone.h>
#include <stdexcept>
#include <vector>
#include <string>

std::vector<Instruction> disassemble(
    const uint8_t* data,
    size_t         size,
    uint64_t       base_address
) {
    csh handle;
    if (cs_open(CS_ARCH_X86, CS_MODE_64, &handle) != CS_ERR_OK) {
        throw std::runtime_error("cs_open failed");
    }

    cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);

    cs_insn* insn;
    size_t count = cs_disasm(handle, data, size, base_address, 0, &insn);

    std::vector<Instruction> result;
    if (count > 0) {
        for (size_t i = 0; i < count; i++) {
            Instruction instr;
            instr.address = insn[i].address;
            instr.mnemonic = insn[i].mnemonic;
            instr.op_str = insn[i].op_str;
            
            instr.bytes.assign(insn[i].bytes, insn[i].bytes + insn[i].size);
            
            result.push_back(instr);

            if (instr.mnemonic == "ret") {
                break;
            }
        }
        cs_free(insn, count);
    }

    cs_close(&handle);
    return result;
}
