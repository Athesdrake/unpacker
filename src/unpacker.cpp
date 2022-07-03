#include "unpacker.hpp"

bool match_target(
    StringFinder& finder, std::string& target, std::unordered_map<uint32_t, char>& methods) {
    uint32_t chr = 0;
    for (const char& c : target) {
        if (!finder.next_char(chr) || methods[chr] != c) {
            finder.skip_string();
            return false;
        }
    }
    return true;
}

std::string get_keymap(std::shared_ptr<AbcFile> abc, std::shared_ptr<Instruction> ins) {
    while (ins) {
        if (ins->opcode == OP::pushstring)
            return abc->cpool.strings[ins->args[0]];

        ins = ins->next;
    }
    return "";
}

std::unordered_map<uint32_t, char> get_methods(std::shared_ptr<AbcFile> abc, std::string keymap) {
    std::unordered_map<uint32_t, char> methods;

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
    return methods;
}

std::vector<std::string> resolve_order(std::shared_ptr<AbcFile> abc) {
    // Get the keymap from the cinit method
    auto parser = Parser(abc->methods[abc->classes[0].cinit]);
    auto keymap = get_keymap(abc, parser.begin);

    // then resolve the methods return value
    auto methods = get_methods(abc, keymap);

    // Resolve the binaries order from the iinit method
    parser   = Parser(abc->methods[abc->classes[0].iinit]);
    auto ins = parser.begin;

    // Skip instructions before super()
    while (ins) {
        if (ins->opcode == OP::constructsuper)
            break;

        ins = ins->next;
    }

    std::vector<std::string> order;
    std::string target = "writeBytes";
    StringFinder finder(ins);

    // Resolve binaries order
    while (finder.next_string()) {
        if (match_target(finder, target, methods)) {
            // The next string is the binary's name
            finder.next_string();
            order.push_back(finder.build(methods));
        }
    }
    return order;
}

std::unordered_map<std::string, swf::DefineBinaryDataTag*> get_binaries(swf::Swf& movie) {
    const auto& symbols = movie.symbol_class->symbols;
    std::unordered_map<std::string, swf::DefineBinaryDataTag*> binaries;

    for (auto& tag : movie.binaries) {
        const auto& it = symbols.find(tag->charId);
        if (it != symbols.end()) {
            // Remove the prefix
            const auto symbol = it->second.substr(it->second.find('_') + 1);
            binaries[symbol]  = tag;
        }
    }
    return binaries;
}
