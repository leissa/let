# Copilot Instructions for Let

## Build Commands

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j $(nproc)
```

Run the interpreter:
```sh
./build/bin/let test/test.let -e      # evaluate
./build/bin/let test/test.let -d      # dump (pretty-print AST)
```

There is no test suite or lint target. CI runs the binary under Valgrind against `test/test.let`.

## Architecture

Let is a small interpreter for a toy language, built on top of the [FE](https://github.com/leissa/fe) compiler framework (included as a git submodule in `external/fe`).

### Pipeline

Source text → **Lexer** → token stream → **Parser** → AST → **eval** / **stream** (dump)

### Key classes and their FE base classes

| Class | FE Base | Role |
|-------|---------|------|
| `Driver` | `fe::Driver` | Owns the arena allocator and symbol table; tracks error count |
| `Lexer` | `fe::Lexer<1, Lexer>` (CRTP) | Tokenizes input using FE's UTF-8–aware character stream |
| `Parser` | `fe::Parser<Tok, Tok::Tag, 1, Parser>` (CRTP) | Pratt-style expression parsing; lookahead of 1 token |
| `Tok` | — | Token with a `Tag` enum, carrying either a `Sym` or `uint64_t` |
| AST nodes (`Expr`, `Stmt`, `Prog`) | `Node` → `fe::RuntimeCast<Node>` | Arena-allocated (`fe::Arena::Ptr<const T>`) |

### FE framework patterns used throughout

- **Arena allocation**: AST nodes are created via `Driver::ast<T>(args...)` which delegates to `fe::Arena::mk`. The resulting smart pointer type is `fe::Arena::Ptr<const T>`, aliased as `AST<T>`.
- **Symbol interning**: Identifiers are interned as `fe::Sym` via `Driver::sym()`. The environment is `fe::SymMap<uint64_t>`.
- **Location tracking**: `fe::Loc` / `fe::Pos` are used everywhere; the parser's `tracker()` captures source ranges.
- **Error reporting**: `driver().err(loc, fmt, ...)` — uses `std::format`-style strings.
- **Lexer helpers**: `accept()`, `next()`, `ahead()`, `start()`, `str_` follow the `fe::Lexer` API.
- **Parser helpers**: `ahead()`, `lex()`, `eat()`, `accept()`, `expect()` follow the `fe::Parser` API.

## Conventions

- **C++20** (C++23 on MSVC). Uses `std::format`, concepts, and structured bindings.
- **X-macros** define token tags and operators (`LET_KEY`, `LET_VAL`, `LET_TOK`, `LET_OP` in `tok.h`). Extend these macros when adding new keywords or operators.
- **Case-insensitive** lexing: identifiers are lowered during lexing (`accept<Append::Lower>`).
- **Namespace**: all code lives in `namespace let`.
- **Headers** in `include/let/`, sources in `src/let/`, with `src/main.cpp` as the entry point.
- **Formatting**: clang-format enforced via pre-commit hook. 4-space indent, 120-column limit, Attach brace style, pointer/ref left-aligned. Use `// clang-format off/on` guards for deliberately formatted tables (see existing usage in `eval.cpp`, `parser.cpp`, `tok.h`).
- **Semantics**: 64-bit unsigned wrap-around arithmetic; division by zero yields zero.
