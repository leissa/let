# Let repository instructions

Let is a tiny interpreter - `let`/`print` statements over `uint64_t` arithmetic - whose purpose is to showcase [FE](https://github.com/leissa/fe), the compiler-frontend library in the `submodules/fe` submodule.
Both have the same author, and changes here often go hand in hand with changes there; `submodules/fe/.github/copilot-instructions.md` documents FE's API and contracts.

## Build & run

```sh
git submodule update --init --recursive   # FE and its own submodules are required
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j $(nproc)
./build/bin/let test/eval.let -e          # -d dumps the parsed program, --help lists the rest
```

C++23 and CMake 3.29 are required.
`cmake --install build --prefix <prefix>` installs only `let` and `LICENSE`: an embedded FE installs nothing by default.

## Tests

Golden-file tests, driven by CTest; there is no test framework.

```sh
ctest --test-dir build --output-on-failure   # -R <regex> for one test
```

`ctest` does not rebuild, but `make test`/`ninja test` does (`CMAKE_SKIP_TEST_ALL_DEPENDENCY FALSE`, which is why CMake 3.29 is required).
`test/CMakeLists.txt` globs the `.let` files (`CONFIGURE_DEPENDS`) and `test/run_test.cmake` runs each one via `cmake -P`, so the same logic works on Windows.
Tests run from the project root because the goldens contain the source path as diagnostics print it.

- `test/<name>.let` (label `eval`): run with `-e`, must exit 0, and stdout must match `test/<name>.out` exactly.
- `test/error/<name>.let` (label `error`): must exit non-zero, and every non-empty, non-`#` line of `test/error/<name>.err` must occur verbatim in stderr - snippet rows included. An optional `<name>.flags` holds extra CLI arguments.
- A missing `.out`/`.err` is *generated* from the current output and the test passes: to add a test, write the `.let`, build, run `ctest`, and review the new golden.
- Captured output lands in `build/test/<label>-<name>.{stdout,stderr}`.

CI builds Debug and Release with gcc-15 (Linux), Apple clang (macOS), and MSVC (Windows).
On Linux, every test - error tests included - also runs under Valgrind, and a separate job runs the suite under ASan/LSan/UBSan (no LSan on macOS).
Error tests exit non-zero by design, so those jobs check a log rather than the exit code: error paths must be leak-free too.

## Architecture

A classic pipeline, one class per stage, each derived from an FE CRTP base:

- `include/let/tok.h` - `Tok`, generated from the X-macros `LET_KEY`, `LET_VAL`, `LET_TOK`, `LET_OP`. A new keyword, token, or operator goes there; its spelling, tag, and precedence (`Tok::Prec`) follow.
- `include/let/driver.h` - `let::Driver : fe::Driver` adds an `fe::Arena` for the AST and the interned keywords: `Driver::keys()` is an `fe::SymTab` built once in the constructor. Intern with `Driver::sym` first, then look up - `SymTab` hashes the interned pointer.
- `src/let/lexer.cpp` - `Lexer : fe::Lexer<1, Lexer>`. The whole source sits in FE's buffer, so `view()` is the token text for free. Keywords are deliberately case-insensitive (`needs_fold(view()) ? sym(lower()) : sym(view())`) to exercise that FE path - keep it.
- `src/let/parser.cpp` - `Parser : fe::Parser<Tok, Tok::Tag, 1, Parser>`, recursive descent with precedence climbing (`parse_expr(ctxt, Prec)`). It uses FE's default diagnostics unchanged: a missing `)` gets its "to match this `(`" note from the `anchor(paren_l, Tag::D_paren_r)` alone.
- `include/let/ast.h`, `src/let/eval.cpp`, `src/let/stream.cpp` - AST nodes with `stream()` (dump) and `eval(Env&)` (tree-walking interpreter, `Env = fe::SymMap<uint64_t>`).
- `src/main.cpp` - the `fe::Cli` options and the pipeline.

Invariants worth knowing:

- `Driver::ast<T>(...)` allocates nodes in the arena as `AST<T> = fe::Arena::Ref<const T>`. Nodes are immutable and **never destroyed** - the arena frees them wholesale - so a node holds only trivially destructible members and declares no destructor.
- A node owning a list keeps it in its own allocation via `fe::VLA<Self>` (see `Prog`): it names the element types in `VLA_Types` and hands each out as `View<T>` via `vla<I>()`. The ranges are the **last** arguments of `Driver::ast<T>()`, in `VLA_Types` order. `ASTs<T> = fe::Vector<AST<T>>` is only the parser's scratch buffer.
- Named nodes (`SymExpr`, `LetStmt`) carry an `fe::Dbg`, so a diagnostic can point at the name rather than the whole node.
- Every stage reports into `driver().error()` (`e`/`w`, plus `n` for a note on the preceding message). Parsing never throws; `main.cpp` calls `driver.error().ack()` afterwards, which throws an `fe::Error::Bail` if anything went wrong - so `eval` only sees well-formed programs. `ack` must run while the `Driver` lives, since every `Loc` points into its `SrcMap`; the `Bail` carries finished text and may be caught anywhere.
- Messages are FE citation markup: put code in backticks, never quotes.
- Semantics: wrap-around `uint64_t` arithmetic, division by zero yields `0`, and an unbound identifier reads as `0` (not an error).

## Keeping docs in sync

- `README.md` holds the grammar, the precedence table, and a copy of the `--help` output - update it when the language or an option changes.
- FE's `docs/README.md` quotes Let as its worked example, including diagnostics and SLOC counts.
- The `project(... VERSION)` line in `CMakeLists.txt` is the only place a version lives; `--version` reads it through `LET_VERSION`/`FE_VERSION`.
- `scripts/release.sh <version>` releases FE and Let in tandem under the same version.

## Style

- clang-format via pre-commit (`.pre-commit-config.yaml`, `.clang-format`); wrap hand-aligned tables in `// clang-format off/on`.
- Everything lives in `namespace let`; headers in `include/let/`, sources in `src/let/`.
- Comments follow FE's rules: scarce, one short sentence on *why*, never restating the code or narrating a change; Doxygen on declarations is fine. Never increase comment density.
