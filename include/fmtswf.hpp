#pragma once
#include <abc/parser/Parser.hpp>
#include <fmt/format.h>
#include <swflib.hpp>

template <> struct fmt::formatter<std::shared_ptr<swf::abc::AbcFile>> {
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) { return ctx.end(); }
    template <typename FormatContext>
    auto format(const std::shared_ptr<swf::abc::AbcFile>& abc, FormatContext& ctx) {
        return format_to(
            ctx.out(),
            "\t[AbcFile] version {major}.{minor}"
            "\n\t\t methods: {methods}"
            "\n\t\t classes: {classes}"
            "\n\t\t scripts: {scripts}"
            "\n\t\t cpool:"
            "\n\t\t\t integers: {integers}"
            "\n\t\t\t uintegers: {uintegers}"
            "\n\t\t\t doubles: {doubles}"
            "\n\t\t\t strings: {strings}"
            "\n\t\t\t namespaces: {namespaces}"
            "\n\t\t\t ns_sets: {ns_sets}"
            "\n\t\t\t multinames: {multinames}",
            "methods"_a    = abc->methods.size(),
            "classes"_a    = abc->classes.size(),
            "scripts"_a    = abc->scripts.size(),
            "integers"_a   = abc->cpool.integers.size(),
            "uintegers"_a  = abc->cpool.uintegers.size(),
            "doubles"_a    = abc->cpool.doubles.size(),
            "strings"_a    = abc->cpool.strings.size(),
            "namespaces"_a = abc->cpool.namespaces.size(),
            "ns_sets"_a    = abc->cpool.ns_sets.size(),
            "multinames"_a = abc->cpool.multinames.size(),
            "major"_a      = abc->major_version,
            "minor"_a      = abc->minor_version);
    }
};
template <> struct fmt::formatter<swf::DoABCTag> {
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) { return ctx.end(); }
    template <typename FormatContext> auto format(swf::DoABCTag& tag, FormatContext& ctx) {
        return format_to(
            ctx.out(),
            "\t[{tagname}:0x{tagid:0>2x}] \"{name}\" lazy:{lazy}\n{abc}\n",
            "abc"_a     = tag.abcfile,
            "tagname"_a = tag.getTagName(),
            "tagid"_a   = uint8_t(tag.getId()),
            "name"_a    = tag.name,
            "lazy"_a    = tag.is_lazy);
    }
};