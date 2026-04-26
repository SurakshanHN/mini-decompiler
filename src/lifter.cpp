#include "lifter.h"
#include <map>
#include <sstream>
#include <iostream>
#include <algorithm>

// Helper to trim whitespace from both ends of a string
std::string trim(const std::string& s) {
    auto start = s.begin();
    while (start != s.end() && std::isspace(*start)) {
        start++;
    }
    auto end = s.end();
    do {
        end--;
    } while (std::distance(start, end) > 0 && std::isspace(*end));
    return std::string(start, end + 1);
}

bool parse_reg_reg(const std::string& op_str, std::string& dst, std::string& src) {
    size_t comma = op_str.find(',');
    if (comma == std::string::npos) return false;

    dst = trim(op_str.substr(0, comma));
    src = trim(op_str.substr(comma + 1));

    if (dst.find('[') != std::string::npos || src.find('[') != std::string::npos) return false;
    if (dst.find("ptr") != std::string::npos || src.find("ptr") != std::string::npos) return false;
    if (std::isdigit(dst[0]) || std::isdigit(src[0])) return false;

    return !dst.empty() && !src.empty();
}

bool parse_mem_store(const std::string& op_str, std::string& slot, std::string& reg) {
    size_t open_bracket = op_str.find('[');
    size_t close_bracket = op_str.find(']');
    size_t comma = op_str.find(',');

    if (open_bracket == std::string::npos || close_bracket == std::string::npos || comma == std::string::npos) {
        return false;
    }

    if (comma < close_bracket) return false;

    slot = op_str.substr(open_bracket, close_bracket - open_bracket + 1);
    reg = trim(op_str.substr(comma + 1));
    return true;
}

bool parse_mem_load(const std::string& op_str, std::string& reg, std::string& slot) {
    size_t comma = op_str.find(',');
    size_t open_bracket = op_str.find('[');
    size_t close_bracket = op_str.find(']');

    if (comma == std::string::npos || open_bracket == std::string::npos || close_bracket == std::string::npos) {
        return false;
    }

    if (open_bracket < comma) return false;

    reg = trim(op_str.substr(0, comma));
    slot = op_str.substr(open_bracket, close_bracket - open_bracket + 1);
    return true;
}

bool parse_add(const std::string& op_str, std::string& reg, int& imm) {
    size_t comma = op_str.find(',');
    if (comma == std::string::npos) return false;

    reg = trim(op_str.substr(0, comma));
    std::string imm_str = trim(op_str.substr(comma + 1));
    
    try {
        imm = std::stoi(imm_str, nullptr, 0);
        return true;
    } catch (...) {
        return false;
    }
}

bool parse_sub(const std::string& op_str, std::string& reg, int& imm) {
    return parse_add(op_str, reg, imm);
}

std::string lift(const std::vector<Instruction>& instructions) {
    std::map<std::string, std::string> reg_vars;
    std::map<std::string, std::string> stack_vars;
    std::string return_expr = "";

    reg_vars["edi"] = "a";
    reg_vars["rdi"] = "a";

    for (const auto& ins : instructions) {
        if (ins.mnemonic == "push" && ins.op_str == "rbp") continue;
        if (ins.mnemonic == "pop" && ins.op_str == "rbp") continue;
        if (ins.mnemonic == "mov" && ins.op_str == "rbp, rsp") continue;

        std::string dst, src, slot, reg;

        if (ins.mnemonic == "mov" && parse_reg_reg(ins.op_str, dst, src)) {
            if (reg_vars.count(src)) {
                reg_vars[dst] = reg_vars[src];
            }
            continue;
        }

        if (ins.mnemonic == "add" && parse_reg_reg(ins.op_str, dst, src)) {
            std::string lhs = reg_vars.count(dst) ? reg_vars[dst] : dst;
            std::string rhs = reg_vars.count(src) ? reg_vars[src] : src;
            reg_vars[dst] = lhs + " + " + rhs;
            continue;
        }

        if (ins.mnemonic == "sub" && parse_reg_reg(ins.op_str, dst, src)) {
            std::string lhs = reg_vars.count(dst) ? reg_vars[dst] : dst;
            std::string rhs = reg_vars.count(src) ? reg_vars[src] : src;
            reg_vars[dst] = lhs + " - " + rhs;
            continue;
        }

        if (ins.mnemonic == "mov" && parse_mem_store(ins.op_str, slot, reg)) {
            if (reg_vars.count(reg)) {
                stack_vars[slot] = reg_vars[reg];
            }
            continue;
        }
        if (ins.mnemonic == "mov" && parse_mem_load(ins.op_str, reg, slot)) {
            if (stack_vars.count(slot)) {
                reg_vars[reg] = stack_vars[slot];
            }
            continue;
        }

        int imm;
        if (ins.mnemonic == "add" && parse_add(ins.op_str, reg, imm)) {
            if (reg_vars.count(reg)) {
                std::string lhs = reg_vars[reg];
                reg_vars[reg] = lhs + " + " + std::to_string(imm);
            }
            continue;
        }

        if (ins.mnemonic == "sub" && parse_sub(ins.op_str, reg, imm)) {
            if (reg_vars.count(reg)) {
                std::string lhs = reg_vars[reg];
                reg_vars[reg] = lhs + " - " + std::to_string(imm);
            }
            continue;
        }

        if (ins.mnemonic == "ret") {
            if (reg_vars.count("eax")) {
                return_expr = reg_vars["eax"];
            } else {
                return_expr = "result";
            }
            break;
        }
    }

    std::ostringstream oss;
    oss << "int func(int a) {\n";
    if (!return_expr.empty()) {
        oss << "    return " << return_expr << ";\n";
    }
    oss << "}";

    return oss.str();
}
