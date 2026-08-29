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

The binary lands in `build/bin/let`. Requires C++23 (CI builds with gcc-14 and clang on Linux, Apple clang and gcc-14 on macOS, MSVC and clang-cl on Windows). Run the interpreter with e.g. `./build/bin/let test/eval.let -e` (`-d` dumps the parsed program, `-e` evaluates it, `--no-snippet` shrinks diagnostics to their header line, `--max-errors <num>` caps how many are reported).

`CMakeLists.txt` sets `FE_LIB=ON` and links `fe-lib`, not `fe`: `fe` alone is header-only and declares - but does not define - the `Pos`/`Loc` streaming and `fe::Snippet` that the diagnostics need.

## Tests

Golden-file tests driven by a shell script (no test framework):

```sh
bash test/run_tests.sh build/bin/let
```

- `test/*.let` + `test/<name>.out`: run with `-e`, expect exit 0, stdout must match `.out` exactly.
- `test/error/*.let` + `test/<name>.err`: expect non-zero exit; each non-empty, non-`#` line of the `.err` file must appear (fixed-string `grep -F`) in stderr. Most goldens include the snippet rows, so they fail if the source excerpt goes missing.
  An optional `test/error/<name>.flags` file holds extra CLI arguments for that one test.
- If a `.out`/`.err` file is missing, the script **generates** it from the current binary output — that's how you add a test: write the `.let` file, run the script, review the generated golden file.
- Run a single test manually: `./build/bin/let test/eval.let -e | diff test/eval.out -`.

CI (`.github/workflows/{linux,macos,windows}.yml`) builds Debug+Release per compiler and runs the test script. On Linux every test — including the error tests — additionally runs under valgrind (`--leak-check=full`), and a separate job runs the whole suite under ASan+LSan+UBSan (ASan+UBSan on macOS, which has no LSan). Since the error tests exit non-zero by design, both check a log rather than the exit code — so avoid leaks in error paths too.

## Architecture

Classic pipeline, one class per stage, all deriving from FE's CRTP base classes:

- **`include/let/tok.h`** — `Tok` (token) built from X-macros (`LET_KEY`, `LET_VAL`, `LET_TOK`, `LET_OP`). Adding a keyword/operator/token means extending these macros; string forms, tags, and operator precedence (`Tok::Prec`) all derive from them.
- **`include/let/driver.h`** — `let::Driver : fe::Driver` holds the symbol table, the `SrcMap`, and an `fe::Arena`; `driver.ast<T>(...)` arena-allocates all AST nodes (returned as `AST<T> = fe::Arena::Ptr<const T>`; nodes are immutable after construction). Diagnostics are *not* in the driver: `main.cpp` creates one `fe::Error` and threads it through `Lexer`/`Parser` (both keep an `fe::Error& err_`), which record messages via `err_.{error,note}`. A note attaches to the error before it: without a `Loc` it renders as a `= note:` continuation, with one it gets its own header line and snippet — and drops itself when that `Loc` overlaps the error's own. `fe::Error` puts an `fe::Snippet` source excerpt under every message and renders `` `code` `` citations in color, so phrase messages with backticks, not quotes; `fe::Driver::diag.{no_snippet,gutter,max_rows,max_errors,werror}` tune the layout and how much is kept.
- **`src/let/lexer.cpp`** — `Lexer : fe::Lexer<1, Lexer>`. Deliberately case-insensitive (folds identifiers/keywords to lower case via `fe::Lexer::accept<Append::Lower>`) to exercise that FE path — keep this behavior.
- **`src/let/parser.cpp`** — `Parser : fe::Parser<Tok, Tok::Tag, 1, Parser>`, recursive descent with precedence climbing (`parse_expr(ctxt, Prec)`). Parse errors don't throw while parsing; they accumulate in the shared `fe::Error`, and `main.cpp` calls `err.ack()` afterwards, which throws an `fe::Error::Bail` if anything was collected — so the eval only ever sees a well-formed program. `ack` must be called while the `Driver` is still alive, because every `Loc` in the `Error` points into its `SrcMap`; the `Bail` it throws carries only the finished text, so *that* may be caught anywhere. A missing `)` gets a located `Error::note` pointing back at the `(`; `parse_primary_or_unary_expr` remembers that `(` in `paren_l_` with an `fe::Restore` that covers the `expect` closing the `fe::Parser::Anchor`'s scope.
- **`include/let/ast.h`** + **`src/let/eval.cpp`** / **`src/let/stream.cpp`** — AST nodes implement `stream()` (dumping) and `eval(Env&)` (tree-walking interpreter, `Env = fe::SymMap<uint64_t>`). Node lists are `ASTs<T> = fe::Vector<AST<T>>` handed out as `View<T> = fe::View<AST<T>>`. Named nodes (`SymExpr`, `LetStmt`) carry an `fe::Dbg` — a `Loc`/`Sym` pair — so a diagnostic can point at the name rather than the whole node. Semantics: wrap-around u64 arithmetic, division by zero yields 0, unbound identifiers read as 0 (not an error).

`main.cpp` parses CLI args by hand (no library). The grammar and precedence table are documented in `README.md` — update it when changing the language.

## Formatting & releases

- clang-format is enforced via pre-commit (`.pre-commit-config.yaml`); `.clang-format` is at the repo root. Code uses `// clang-format off/on` around the X-macro tables.
- `scripts/release.sh <version>` releases fe and let in tandem with the same version number (bumps `project(... VERSION)`, tags, pushes, creates GitHub releases via `gh`).
