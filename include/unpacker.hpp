#pragma once
#include "string_finder.hpp"
#include <abc/parser/Parser.hpp>
#include <swflib.hpp>

using namespace swf::abc::parser;
using AbcFile = swf::abc::AbcFile;

constexpr const char* version = "0.1.1";

bool match_target(
    StringFinder& finder, std::string& target, std::unordered_map<uint32_t, char>& methods);

std::string get_keymap(std::shared_ptr<AbcFile> abc, std::shared_ptr<Instruction> ins);
std::unordered_map<uint32_t, char> get_methods(std::shared_ptr<AbcFile> abc, std::string keymap);
std::vector<std::string> resolve_order(std::shared_ptr<AbcFile> abc);
std::unordered_map<std::string, swf::DefineBinaryDataTag*> get_binaries(swf::Swf& movie);