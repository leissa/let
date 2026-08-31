#include <cstring>

#include <iostream>
#include <stdexcept>

#include <fe/error.h>

#include "let/parser.h"

using namespace std::literals;

int main(int argc, char** argv) {
    try {
        static const auto version = "let " LET_VERSION "\n";
        static const auto usage   = "USAGE:\n"
                                    "  let [-?|-h|--help] [-v|--version] [-d|--dump] [-e|--eval]\n"
                                    "      [--max-errors <num>] [--no-snippet] [<file>]\n"
                                    "\n"
                                    "Display usage information.\n"
                                    ""
                                    "OPTIONS, ARGUMENTS:\n"
                                    "  -?, -h, --help"
                                    "  -v, --version           Display version info and exit.\n"
                                    "  -d, --dump              Dumps the let program again.\n"
                                    "  -e, --eval              Evaluate the let program.\n"
                                    "      --max-errors <num>  Report at most <num> errors; 0 reports all of them.\n"
                                    "      --no-snippet        Only emit the header line of a diagnostic.\n"
                                    "  <file>                  Input file.\n";
        bool dump                 = false;
        bool eval                 = false;
        bool no_snippet           = false;
        uint32_t max_errors       = 0;
        std::string input;

        for (int i = 1; i < argc; ++i) {
            if (argv[i] == "-v"s || argv[i] == "--version"s) {
                std::cout << version;
                return EXIT_SUCCESS;
            } else if (argv[i] == "-?"s || argv[i] == "-h"s || argv[i] == "--help"s) {
                std::cerr << usage;
                return EXIT_SUCCESS;
            } else if (argv[i] == "-d"s || argv[i] == "--dump"s) {
                dump = true;
            } else if (argv[i] == "-e"s || argv[i] == "--eval"s) {
                eval = true;
            } else if (argv[i] == "--max-errors"s) {
                if (++i == argc) throw std::invalid_argument("--max-errors requires a number");
                max_errors = uint32_t(std::stoul(argv[i]));
            } else if (argv[i] == "--no-snippet"s) {
                no_snippet = true;
            } else {
                if (!input.empty()) throw std::invalid_argument("more than one input file given");
                input = argv[i];
            }
        }

        if (input.empty()) throw std::invalid_argument("no input given");

        auto driver              = let::Driver();
        driver.diag().no_snippet = no_snippet;
        driver.diag().max_errors = max_errors;
        auto path                = std::filesystem::path(input);
        auto src                 = driver.src().add(path).first;
        if (!src) throw std::runtime_error(std::format("cannot read file \"{}\"", input));
        auto parser = let::Parser(driver, *src);
        auto prog   = parser.parse_prog();

        if (dump) prog->dump();

        driver.error().ack();   // throws what it collected; merely reports the warnings
        if (eval) prog->eval(); // only evaluate a well-formed program
    } catch (const fe::Error::Bail& bail) {
        std::cerr << bail; // already rendered, so the Driver it came from may be long gone
        return EXIT_FAILURE;
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "error: unknown exception" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
