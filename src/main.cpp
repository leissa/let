#include <cstring>

#include <fstream>
#include <iostream>
#include <stdexcept>

#include "let/parser.h"

using namespace std::literals;

int main(int argc, char** argv) {
    try {
        static const auto version = "let 0.1\n";
        static const auto usage   = "USAGE:\n"
                                    "  let [-?|-h|--help] [-v|--version] [-d|--dump] [-e|--eval] [<file>]\n"
                                    "\n"
                                    "Display usage information.\n"
                                    ""
                                    "OPTIONS, ARGUMENTS:\n"
                                    "  -?, -h, --help"
                                    "  -v, --version           Display version info and exit.\n"
                                    "  -d, --dump              Dumps the let program again.\n"
                                    "  -e, --eval              Evaluate the let program.\n"
                                    "  <file>                  Input file.\n";
        bool dump                 = false;
        bool eval                 = false;
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
            } else {
                if (!input.empty()) throw std::invalid_argument("more than one input file given");
                input = argv[i];
            }
        }

        if (input.empty()) throw std::invalid_argument("no input given");

        auto driver = let::Driver();
        auto path   = std::filesystem::path(input);
        auto ifs    = std::ifstream(path);
        if (!ifs) throw std::runtime_error(std::format("cannot read file \"{}\"", input));
        auto parser = let::Parser(driver, ifs, &path);
        auto prog   = parser.parse_prog();

        if (dump) prog->dump();
        if (eval) prog->eval();

        if (auto num = driver.num_errors()) {
            std::cerr << num << " error(s) encountered" << std::endl;
            return EXIT_FAILURE;
        }
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "error: unknown exception" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
