#include "unpacker.hpp"
#include <cpr/cpr.h>
#include <cstring>

namespace athes::unpack {
void download(std::string url, std::vector<uint8_t>& buffer) {
    auto r = cpr::Get(
        cpr::Url { url },
        cpr::WriteCallback([&buffer](const std::string_view& data, intptr_t userdata) {
            buffer.insert(buffer.end(), data.begin(), data.end());
            return true;
        }));

    if (r.error.code != cpr::ErrorCode::OK)
        throw std::runtime_error(r.error.message);
}

Unpacker::Unpacker(std::unique_ptr<swf::StreamReader> stream)
    : buffer(), stream(std::move(stream)) {
    order    = {};
    binaries = {};
}

Unpacker::Unpacker(std::vector<uint8_t> buffer) : buffer(buffer) {
    stream   = std::make_unique<swf::StreamReader>(buffer);
    order    = {};
    binaries = {};
}

Unpacker::Unpacker(std::string url) : buffer() {
    download(url, buffer);
    stream   = std::make_unique<swf::StreamReader>(buffer);
    order    = {};
    binaries = {};
}

swf::StreamWriter Unpacker::unpack() {
    swf::StreamWriter writer;
    read_movie();
    resolve_order();
    resolve_binaries();
    write_binaries(writer);
    return writer;
}

bool Unpacker::unpack(swf::Swf& movie, std::unique_ptr<swf::StreamReader>& stream) {
    auto writer = unpack();
    if (writer.size() == 0) {
        // file was not unpacked
        // move the movie, as we didn't unpack it
        // move also the stream back, as we might still have references to the data from the movie
        movie  = std::move(this->movie);
        stream = std::move(this->stream);
        return false;
    }

    uint8_t* buffer = new uint8_t[writer.size()];
    std::memcpy(buffer, writer.get_buffer(), writer.size());
    stream = std::make_unique<swf::StreamReader>(buffer, buffer + writer.size());
    return true;
}

const size_t Unpacker::size() {
    if (!stream)
        return 0;
    return stream->size();
}

bool Unpacker::has_frame1() { return movie.abcfiles.find("frame1") != movie.abcfiles.end(); }

swf::DoABCTag* Unpacker::get_frame1() {
    if (!has_frame1())
        return nullptr;

    return movie.abcfiles.find("frame1")->second;
}

void Unpacker::read_movie() {
    if (!stream)
        throw std::runtime_error("Stream is not set.");

    movie.read(*stream);
}

void Unpacker::resolve_order() {
    if (!has_frame1())
        return;

    abc = get_frame1()->abcfile;
    Parser parser(abc->methods[abc->classes[0].cinit]);
    // Get the keymap from the cinit method
    // then resolve the methods return value
    resolve_keymap(parser.begin);
    resolve_methods();

    // Resolve the binaries order from the iinit method
    parser   = Parser(abc->methods[abc->classes[0].iinit]);
    auto ins = parser.begin;

    // Skip instructions before super()
    while (ins) {
        if (ins->opcode == OP::constructsuper)
            break;

        ins = ins->next;
    }

    std::string target = "writeBytes";
    StringFinder finder(ins);

    // Resolve binaries order
    while (finder.next_string()) {
        if (match_target(finder, target)) {
            // The next string is the binary's name
            finder.next_string();
            order.push_back(finder.build(methods));
        }
    }
}

void Unpacker::resolve_binaries() {
    if (movie.symbol_class == nullptr)
        return;

    const auto& symbols = movie.symbol_class->symbols;
    for (auto& tag : movie.binaries) {
        const auto& it = symbols.find(tag->charId);
        if (it != symbols.end()) {
            // Remove the prefix
            const auto symbol = it->second.substr(it->second.find('_') + 1);
            binaries[symbol]  = tag;
        }
    }
}

std::optional<std::string> Unpacker::write_binaries(std::ostream& file) {
    for (auto& name : order) {
        const auto& it = binaries.find(name);
        if (it == binaries.end())
            return name;

        auto data = it->second->getData();
        file.write(reinterpret_cast<const char*>(data->raw()), data->size());
    }
    return {};
}
std::optional<std::string> Unpacker::write_binaries(swf::StreamWriter& stream) {
    for (auto& name : order) {
        const auto& it = binaries.find(name);
        if (it == binaries.end())
            return name;

        stream.write(*it->second->getData());
    }
    return {};
}

bool Unpacker::match_target(StringFinder& finder, std::string& target) {
    uint32_t chr = 0;
    for (const char& c : target) {
        if (!finder.next_char(chr) || methods[chr] != c) {
            finder.skip_string();
            return false;
        }
    }
    return true;
}

void Unpacker::resolve_keymap(std::shared_ptr<Instruction> ins) {
    while (ins) {
        if (ins->opcode == OP::pushstring) {
            keymap = abc->cpool.strings[ins->args[0]];
            break;
        }
        ins = ins->next;
    }
}

void Unpacker::resolve_methods() {
    // Get all methods taking a ...rest argument
    // Those methods return a single character from the keymap
    for (auto& trait : abc->classes[0].itraits) {
        if (trait.kind == swf::abc::TraitKind::Method) {
            auto& method = abc->methods[trait.index];
            if (method.need_rest() && method.max_stack == 2) {
                Parser p(method);
                auto ins = p.begin;

                // Get the returned character
                while (ins) {
                    if (ins->opcode == OP::pushbyte) {
                        methods[trait.name] = keymap[ins->args[0]];
                        break;
                    }
                    ins = ins->next;
                }
            }
        }
    }
}
}
