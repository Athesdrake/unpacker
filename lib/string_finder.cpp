#include "string_finder.hpp"

namespace athes::unpack {
StringFinder::StringFinder(std::shared_ptr<Instruction> ins) : ins(ins) { }

bool StringFinder::is_string() {
    return ins->opcode == OP::getlocal0 && ins->next->opcode == OP::callproperty;
}

bool StringFinder::is_add_string() { return is_string() || ins->opcode == OP::add; }
bool StringFinder::is_next_add_string() {
    return (ins->next->opcode == OP::getlocal0 && ins->next->next->opcode == OP::callproperty)
        || ins->next->opcode == OP::add;
}

bool StringFinder::next_string() {
    while (ins) {
        if (is_string())
            return true;
        ins = ins->next;
    }
    return false;
}

bool StringFinder::next_char(uint32_t& chr) {
    if (!is_add_string())
        return false;

    while (ins->opcode == OP::add)
        ins = ins->next;

    ins = ins->next;
    chr = ins->args[0];
    ins = ins->next;
    return true;
}

void StringFinder::skip_string() {
    while (is_add_string()) {
        if (ins->opcode != OP::add)
            ins = ins->next;

        ins = ins->next;
    }
}

uint32_t StringFinder::addr() { return ins->addr; }

std::string StringFinder::build(std::unordered_map<uint32_t, char> methods) {
    std::string str;
    uint32_t chr;

    if (!is_string())
        next_string();

    while (next_char(chr)) {
        auto it = methods.find(chr);
        if (it != methods.end())
            str.push_back(it->second);
    }

    return str;
}
}
