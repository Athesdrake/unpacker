#include "fmtswf.hpp"
#include "unpacker.hpp"
#include "utils.hpp"
#include <argparse/argparse.hpp>
#include <fstream>
#include <iostream>

using namespace swf::abc::parser;
using namespace fmt::literals;
using namespace athes::unpack;
namespace arg   = argparse;
namespace utils = athes::utils;

static utils::Logger logger;

int main(int argc, char const* argv[]) {
    int verbosity = 0;
    arg::ArgumentParser program("unpacker", athes::unpack::version, arg::default_arguments::help);
    program.add_description("Unpack Transformice SWF file.");
    program.add_argument("-V", "--version")
        .action([](const auto&) {
            std::cout << athes::unpack::version << std::endl;
            std::exit(0);
        })
        .default_value(false)
        .help("prints version information and exits")
        .implicit_value(true)
        .nargs(0);
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
        logger.error("{}\n", err.what(), program.help().str());
        return 1;
    }

    /* verbosity 0  -> warning
                 1  -> info
                 2> -> debug
    */
    logger.level = utils::LogLevel(std::max(3 - verbosity, 1) * 10);

    const auto input  = program.get("-i");
    const auto output = program.get("output");
    const bool is_url = input.substr(0, 7) == "http://" || input.substr(0, 8) == "https://";

    utils::TimePoints tps = { { "start", utils::now() } };
    std::unique_ptr<Unpacker> unp;

    const auto& timeit = [&](std::string action, void (Unpacker::*func)()) {
        logger.info("{}. ", action);
        ((*unp).*func)();
        logger.log_done(tps, action);
    };

    auto action = fmt::format("{} file", is_url ? "Downloading" : "Reading");
    logger.info("{} {}. ", action, input);

    try {
        if (is_url) {
            unp = std::make_unique<Unpacker>(input);
        } else if (input == "-") {
            std::vector<uint8_t> buffer;
            utils::read_from_stdin(buffer);
            unp = std::make_unique<Unpacker>(buffer);
        } else {
            unp = std::make_unique<Unpacker>(swf::StreamReader::fromfile(input));
        }
    } catch (const std::exception& err) {
        logger.error("Error: {}\n", err.what());
        return 2;
    }

    logger.log_done(tps, action);
    logger.info(
        "File size: {}\n",
        utils::fmt_unit({ "B", "kB", "MB", "GB" }, static_cast<double>(unp->size())));

    timeit("Parsing file", &Unpacker::read_movie);
    if (!unp->has_frame1()) {
        logger.error("Invalid SWF: Frame1 is not available.\n");
        return 2;
    }
    logger.info("Found frame1:\n{}\n", *unp->get_frame1());

    timeit("Resolving order", &Unpacker::resolve_order);
    if (unp->order.empty()) {
        logger.error("Unable to resolve binaries order. Is it already unpacked?\n");
        return 2;
    }
    logger.info("Order: {}\n", fmt::join(unp->order, ", "));

    timeit("Resolving binaries", &Unpacker::resolve_binaries);
    logger.info("Writing to file {} ", output);

    // Write binaries in the right order to the output file
    std::optional<std::string> missing_binary;
    if (output == "-") {
        if (std::ferror(std::freopen(nullptr, "wb", stdout)))
            throw std::runtime_error(std::strerror(errno));

        missing_binary = unp->write_binaries(std::cout);
    } else {
        std::ofstream file(output, std::ios::binary);
        missing_binary = unp->write_binaries(file);
    }

    logger.log_done(tps, "Writing to file");
    if (missing_binary) {
        logger.error("Unable to find binary with name: {}", *missing_binary);
        return 2;
    }

    logger.display_statistics(tps);
    return 0;
}
