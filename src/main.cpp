#include "unpacker.hpp"
#include "utils.hpp"
#include <cpr/cpr.h>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#define ARGPARSE_LONG_VERSION_ARG_ONLY
#include <argparse/argparse.hpp>

using namespace swf::abc::parser;
using namespace fmt::literals;
namespace arg = argparse;

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

bool write_binaries(
    std::ostream& file, std::vector<std::string> order,
    std::unordered_map<std::string, swf::DefineBinaryDataTag*> binaries) {
    for (auto& name : order) {
        const auto& it = binaries.find(name);
        if (it == binaries.end()) {
            utils::log_error("Unable to find binary with name: {}", name);
            return false;
        }

        auto data = it->second->getData();
        file.write(reinterpret_cast<const char*>(data->raw()), data->size());
    }
    return true;
}

void download(std::string url, std::vector<uint8_t>& buffer) {
    auto r = cpr::Get(
        cpr::Url { url }, cpr::WriteCallback([&buffer](std::string data, intptr_t userdata) {
            buffer.insert(buffer.end(), data.begin(), data.end());
            return true;
        }));

    if (r.error.code != cpr::ErrorCode::OK)
        throw std::runtime_error(r.error.message);
}

int main(int argc, char const* argv[]) {
    int verbosity = 0;
    arg::ArgumentParser program("unpacker", version);
    program.add_description("Unpack Transformice SWF file.");
    program.add_argument("-v", "--verbose")
        .help("Increase output verbosity. Verbose messages go to stderr.")
        .action([&verbosity](const auto& v) { ++verbosity; })
        .append()
        .default_value(0)
        .nargs(0);
    program.add_argument("-i")
        .help("The file url to unpack. Can be a file from the filesystem or an url to download.")
        .default_value(std::string { "https://www.transformice.com/Transformice.swf" });
    program.add_argument("output").help("The ouput file.").required();

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        utils::log_error("{}\n", err.what(), program.help().str());
        return 1;
    }

    utils::log_level  = verbosity;
    const auto input  = program.get("-i");
    const auto output = program.get("output");
    const bool is_url = input.substr(0, 7) == "http://" || input.substr(0, 8) == "https://";

    utils::TimePoints tps = { { "start", utils::now() } };

    std::unique_ptr<swf::StreamReader> stream;
    std::vector<uint8_t> buffer;

    auto action = fmt::format("{} file", is_url ? "Downloading" : "Reading)");
    utils::log_info("{} {}. ", action, input);

    try {
        if (is_url) {
            download(input, buffer);
            stream = std::make_unique<swf::StreamReader>(buffer);
        } else if (input == "-") {
            utils::read_from_stdin(buffer);
            stream = std::make_unique<swf::StreamReader>(buffer);
        } else {
            stream = std::unique_ptr<swf::StreamReader>(swf::StreamReader::fromfile(input));
        }
    } catch (const std::exception& err) {
        utils::log_error("Error: {}\n", err.what());
        return 2;
    }

    utils::log_done(tps, action);
    utils::log_info(
        "File size: {}\n",
        utils::fmt_unit({ "B", "kB", "MB", "GB" }, static_cast<double>(stream->size())));
    utils::log_info("Parsing file. ");

    swf::Swf movie;
    movie.read(*stream);

    utils::log_done(tps, "Parsing file");

    auto frame1 = movie.abcfiles.find("frame1");
    if (frame1 == movie.abcfiles.end()) {
        utils::log_error("Invalid SWF: Frame1 is not available.\n");
        return 2;
    }
    utils::log_info("Found frame1:\n{}\n", *frame1->second);
    utils::log_info("Resolving order. ");

    auto order    = resolve_order(frame1->second->abcfile);
    auto binaries = get_binaries(movie);

    utils::log_done(tps, "Resolving order");
    utils::log_info("Order: {}\n", fmt::join(order, ", "));
    utils::log_info("Writing to file {} ", output);

    // Write binaries in the right order to the output file
    if (output == "-") {
        if (std::ferror(std::freopen(nullptr, "wb", stdout)))
            throw std::runtime_error(std::strerror(errno));

        if (!write_binaries(std::cout, order, binaries))
            return 2;
    } else {
        std::ofstream file(output, std::ios::binary);
        if (!write_binaries(file, order, binaries))
            return 2;
    }

    utils::log_done(tps, "Writing to file");

    if (verbosity > 1) {
        utils::log("Timing stats:\n");

        auto it   = tps.begin();
        auto prev = &it->second;

        while (++it != tps.end()) {
            auto took = utils::elapsled(*prev, it->second);
            utils::log(
                " - {action}: {took}\n",
                "action"_a = it->first,
                "took"_a   = utils::fmt_unit({ "µs", "ms", "s" }, took, 1000));
            prev = &it->second;
        }
        auto total = utils::elapsled(tps.front().second, tps.back().second);
        utils::log("Total: {}\n", utils::fmt_unit({ "µs", "ms", "s" }, total, 1000));
    }
    return 0;
}
