#include <cstring>

#include <fstream>
#include <iostream>
#include <lyra/lyra.hpp>

#include "let/parser.h"

using namespace std::literals;

int main(int argc, char** argv) {
    try {
        // TODO put version number into cmake magic
        static const auto version = "let 0.1\n";
        bool show_help            = false;
        bool show_version         = false;
        bool dump                 = false;
        std::string input;

        // clang-format off
        auto cli = lyra::cli()
            | lyra::help(show_help)
            | lyra::opt(show_version       )["-v"]["--version"]("Display version info and exit.")
            | lyra::opt(dump               )["-d"]["--dump"   ]("Dumps the let statement again.")
            | lyra::arg(input,       "file")                   ("Input file.")
            ;
        // clang-format on

        if (auto result = cli.parse({argc, argv}); !result) throw std::invalid_argument(result.message());

        if (show_help) {
            std::cout << cli << std::endl;
            std::cout << "Use \"-\" as <file> to output to stdout." << std::endl;
            return EXIT_SUCCESS;
        }

        if (show_version) {
            std::cerr << version;
            return EXIT_SUCCESS;
        }

        if (input.empty()) throw std::invalid_argument("error: no input given");

        auto path = std::filesystem::path(input);
        auto ifs  = std::ifstream(path);
        if (!ifs) {
            // errln("error: cannot read file '{}'", input);
            return EXIT_FAILURE;
        }

        fe::Driver driver;
        let::Ptr<let::Prog> prog;
        if (input == "-") {
            let::Parser parser(driver, std::cin);
            prog = parser.parse_prog();
        } else {
            std::ifstream ifs(input);
            let::Parser parser(driver, ifs, &path);
            prog = parser.parse_prog();
        }

        if (dump) prog->dump();

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
