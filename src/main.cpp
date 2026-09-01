#include <format>
#include <iostream>
#include <stdexcept>

#include <fe/cli.h>
#include <fe/error.h>

#include "let/parser.h"

int main(int argc, char** argv) {
    try {
        bool show_help = false, show_version = false, dump = false, eval = false, no_snippet = false;
        uint32_t max_errors = 0;
        std::string input;

        auto cli = fe::Cli("let", "A simple demo language that builds upon FE.");
        cli.help(show_help)
            .opt(show_version, "-v", "--version", "Display version info and exit.")
            .opt(dump, "-d", "--dump", "Dumps the let program again.")
            .opt(eval, "-e", "--eval", "Evaluate the let program.")
            .opt(max_errors, "num", "", "--max-errors", "Report at most <num> errors; 0 reports all of them.")
            .opt(no_snippet, "", "--no-snippet", "Only emit the header line of a diagnostic.")
            .arg(input, "file", "Input file.");

        if (auto err = cli.parse(argc, argv)) throw std::invalid_argument(*err);

        if (show_help) {
            std::cerr << cli;
            return EXIT_SUCCESS;
        }

        if (show_version) {
            std::cout << "let " LET_VERSION "\n";
            return EXIT_SUCCESS;
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
