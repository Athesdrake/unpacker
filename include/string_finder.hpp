#pragma once
#include <abc/parser/Parser.hpp>
#include <unordered_map>

using namespace swf::abc::parser;

class StringFinder {
    std::shared_ptr<Instruction> ins;

public:
    StringFinder(std::shared_ptr<Instruction> ins);

    bool is_string();
    bool is_add_string();
    bool is_next_add_string();
    bool next_string();
    bool next_char(uint32_t& chr);
    uint32_t addr();
    void skip_string();
    std::string build(std::unordered_map<uint32_t, char> methods);
};
