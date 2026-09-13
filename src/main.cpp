#include <format>
#include <iostream>

#include <fe/cli.h>
#include <fe/error.h>

#include "let/parser.h"

int main(int argc, char** argv) {
    // fe::CodeDiag renders a diagnostic when it is *recorded*, so decide on color up front.
    fe::term::resolve_mode();
    let::Driver driver; // outlives the handler below: it writes into the Driver's Diag

    try {
        // TODO put version number into cmake magic
        bool show_help = false, show_version = false, dump = false, eval = false;
        std::string input;

        auto loc_style = [&](const std::string& t) -> std::string {
            // clang-format off
            if      (t == "full"  ) driver.diag().loc_style = fe::Loc::Style::Full;
            else if (t == "rowcol") driver.diag().loc_style = fe::Loc::Style::RowCol;
            else if (t == "row"   ) driver.diag().loc_style = fe::Loc::Style::Row;
            else if (t == "msvc"  ) driver.diag().loc_style = fe::Loc::Style::MSVC;
            else return std::format("'{}' is not a location style", t);
            // clang-format on
            return {};
        };

        // clang-format off
        auto cli = fe::Cli("sql", "libsql command-line utility.")
            .help(show_help)
            .opt(show_version           ,          "-v", "--version"   , "Display version info and exit.")
            .opt(dump                   ,          "-d", "--dump"      , "Dumps the Let program again.")
            .opt(eval                   ,          "-e", "--eval"      , "Evaluate program.")
            .grp("Diagnostics")
            .opt(loc_style              , "style", ""  , "--loc-style" , "How a diagnostic spells out a source location: `full` (`path:row:col-row:col`), `rowcol` (`path:row:col`), `row` (`path:row`), or msvc (`path(row,col)`).")
            .opt(driver.diag().no_snippet,         ""  , "--no-snippet", "Does not render the offending source line and caret underneath a diagnostic.")
            .opt(driver.diag().gutter   , "width", ""  , "--gutter"    , "Width of a diagnostic's line-number column.")
            .opt(driver.diag().max_rows , "num"  , ""  , "--max-rows"  , "Maximum number of rows a diagnostic's snippet renders before eliding its middle; `0` elides nothing.")
            .opt(driver.diag().max_errors,"num"  , ""  , "--max-errors", "Maximum number of errors to report before dropping the rest; `0` reports all of them.")
            .opt(driver.diag().werror   ,          ""  , "--werror"    , "Treats warnings as errors.")
            .arg(input, "file", "Input file.")
            .epilog("Use \"-\" as `<file>` to output to stdout.");
        // clang-format on

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
