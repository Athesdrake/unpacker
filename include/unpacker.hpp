#pragma once
#include "string_finder.hpp"
#include <abc/parser/Parser.hpp>
#include <optional>
#include <swflib.hpp>

namespace athes::unpack {
using namespace swf::abc::parser;
using AbcFile = swf::abc::AbcFile;

constexpr const char* version = "0.2.2";

class Unpacker {
public:
    swf::Swf movie;
    std::vector<std::string> order;
    std::unordered_map<std::string, swf::DefineBinaryDataTag*> binaries;

    Unpacker(std::unique_ptr<swf::StreamReader> stream);
    Unpacker(std::vector<uint8_t> buffer);
    Unpacker(std::string url);

    const size_t size();
    bool has_frame1();
    swf::DoABCTag* get_frame1();

    /**
     * Unpack the movie if needed and store the result into the params.
     * Return true when the file was successfully unpacked.
     */
    bool unpack(swf::Swf& movie, std::unique_ptr<swf::StreamReader>& stream);
    /**
     * Unpack the movie and return a buffer with the unpacked SWF.
     * The output is empty when it was unable to unpack it.
     */
    swf::StreamWriter unpack();
    void read_movie();
    void resolve_order();
    void resolve_binaries();

    std::optional<std::string> write_binaries(std::ostream& file);
    std::optional<std::string> write_binaries(swf::StreamWriter& stream);

protected:
    bool match_target(StringFinder& finder, std::string& target);
    void resolve_keymap(std::shared_ptr<Instruction> ins);
    void resolve_methods();

    std::shared_ptr<AbcFile> abc;
    std::unique_ptr<swf::StreamReader> stream;
    std::vector<uint8_t> buffer;

    std::string keymap;
    std::unordered_map<uint32_t, char> methods;
};

bool match_target(
    StringFinder& finder, std::string& target, std::unordered_map<uint32_t, char>& methods);

std::string get_keymap(std::shared_ptr<AbcFile> abc, std::shared_ptr<Instruction> ins);
std::unordered_map<uint32_t, char> get_methods(std::shared_ptr<AbcFile> abc, std::string keymap);
}
