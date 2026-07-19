# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

Let is a small demo language (interpreter for `let`/`print` statements over unsigned 64-bit arithmetic) whose purpose is to showcase the [FE](https://leissa.github.io/fe/) compiler-frontend library, which lives in `submodules/fe` and is maintained by the same author. Changes here often go hand in hand with changes in the fe submodule.

## Build

```sh
git clone --recurse-submodules <url>   # fe submodule is required
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j $(nproc)
```

The binary lands in `build/bin/let`. Requires C++23 (CI uses g++-13). Run the interpreter with e.g. `./build/bin/let test/eval.let -e` (`-d` dumps the parsed program, `-e` evaluates it).

## Tests

Golden-file tests driven by a shell script (no test framework):

```sh
bash test/run_tests.sh build/bin/let
```

- `test/*.let` + `test/<name>.out`: run with `-e`, expect exit 0, stdout must match `.out` exactly.
- `test/error/*.let` + `test/<name>.err`: expect non-zero exit; each non-empty, non-`#` line of the `.err` file must appear (fixed-string `grep -F`) in stderr.
- If a `.out`/`.err` file is missing, the script **generates** it from the current binary output — that's how you add a test: write the `.let` file, run the script, review the generated golden file.
- Run a single test manually: `./build/bin/let test/eval.let -e | diff test/eval.out -`.

CI (`.github/workflows/{linux,macos,windows}.yml`) builds Debug+Release, runs the test script, and on Linux also runs the binary under valgrind with `--error-exitcode=1 --leak-check=full` — avoid leaks even in error paths.

## Architecture

Classic pipeline, one class per stage, all deriving from FE's CRTP base classes:

- **`include/let/tok.h`** — `Tok` (token) built from X-macros (`LET_KEY`, `LET_VAL`, `LET_TOK`, `LET_OP`). Adding a keyword/operator/token means extending these macros; string forms, tags, and operator precedence (`Tok::Prec`) all derive from them.
- **`include/let/driver.h`** — `let::Driver : fe::Driver` holds the symbol table, error counting, and an `fe::Arena`; `driver.ast<T>(...)` arena-allocates all AST nodes (returned as `AST<T> = fe::Arena::Ptr<const T>`; nodes are immutable after construction).
- **`src/let/lexer.cpp`** — `Lexer : fe::Lexer<1, Lexer>`. Deliberately case-insensitive (folds identifiers/keywords to lower case via `fe::Lexer::accept<Append::Lower>`) to exercise that FE path — keep this behavior.
- **`src/let/parser.cpp`** — `Parser : fe::Parser<Tok, Tok::Tag, 1, Parser>`, recursive descent with precedence climbing (`parse_expr(ctxt, Prec)`). Parse errors don't throw; they increment the driver's error count, and `main.cpp` refuses to eval if `driver.num_errors() != 0`.
- **`include/let/ast.h`** + **`src/let/eval.cpp`** / **`src/let/stream.cpp`** — AST nodes implement `stream()` (dumping) and `eval(Env&)` (tree-walking interpreter, `Env = fe::SymMap<uint64_t>`). Semantics: wrap-around u64 arithmetic, division by zero yields 0, unbound identifiers read as 0 (not an error).

`main.cpp` parses CLI args by hand (no library). The grammar and precedence table are documented in `README.md` — update it when changing the language.

## Formatting & releases

- clang-format is enforced via pre-commit (`.pre-commit-config.yaml`); `.clang-format` is at the repo root. Code uses `// clang-format off/on` around the X-macro tables.
- `scripts/release.sh <version>` releases fe and let in tandem with the same version number (bumps `project(... VERSION)`, tags, pushes, creates GitHub releases via `gh`).
