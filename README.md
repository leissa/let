# Let

[![linux](https://img.shields.io/github/actions/workflow/status/leissa/let/linux.yml?logo=linux&logoColor=white&label=linux&link=https%3A%2F%2Fgithub.com%2Fleissa%2Flet%2Factions%2Fworkflows%2Flinux.yml)](https://github.com/leissa/let/actions/workflows/linux.yml)
[![macos](https://img.shields.io/github/actions/workflow/status/leissa/let/macos.yml?logo=apple&logoColor=white&label=macos&link=https%3A%2F%2Fgithub.com%2Fleissa%2Flet%2Factions%2Fworkflows%2Fmacos.yml)](https://github.com/leissa/let/actions/workflows/macos.yml)
[![windows](https://img.shields.io/github/actions/workflow/status/leissa/let/windows.yml?logo=windows&logoColor=white&label=windows&link=https%3A%2F%2Fgithub.com%2Flet%2Fleissa%2Factions%2Fworkflows%2Fwindows.yml)](https://github.com/leissa/let/actions/workflows/windows.yml)

A simple demo language that builds upon [FE](https://leissa.github.io/fe/).

## Usage

```
Usage: let [options] <file>

A simple demo language that builds upon FE.

Arguments:
  <file>                  Input file.

Options:
  -h, --help              Display this help and exit.
  -v, --version           Display version info and exit.
  -d, --dump              Dumps the let program again.
  -e, --eval              Evaluate the let program.
      --max-errors <num>  Report at most <num> errors; 0 reports all of them.
                          [default: 0]
      --no-snippet        Only emit the header line of a diagnostic.
```

## Diagnostics

Diagnostics are collected in an `fe::Error` that the lexer and the parser share; `main` hands it to `fe::Error::ack`, which throws it once parsing is done.
Each message prints the offending source row and underlines the columns the `fe::Loc` covers, renders its `` `code` `` citations in color, and may carry notes pointing at a related location:

```
test/error/unclosed_paren.let:1:13: error: expected `)`, got `;` while parsing parenthesized expression
    1 | print (1 + 2;
      |             ^
      test/error/unclosed_paren.let:1:7: note: unmatched `(` opened here
    1 | print (1 + 2;
      |       ^
```

`--no-snippet` sets `fe::Driver::diag.no_snippet`, which drops the rows underneath and leaves just the header lines.

## Building

If you have a [GitHub account setup with SSH](https://docs.github.com/en/authentication/connecting-to-github-with-ssh), just do this:
```sh
git clone --recurse-submodules git@github.com:leissa/let.git
```
Otherwise, clone via HTTPS:
```sh
git clone --recurse-submodules https://github.com/leissa/let.git
```
Then, build with:
```sh
cd let
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j $(nproc)
```
For a `Release` build simply use `-DCMAKE_BUILD_TYPE=Release`.

Invoke the interpreter like so:
```sh
./build/bin/let test/test.let -e
```

Run the test suite via CTest:
```sh
ctest --test-dir build
```

## Grammar

```ebnf
p = s ... s EOF         (* program *)
  ;

s = ';'                 (* empty statement *)
  | 'let' ID '=' e ';'  (* let statement *)
  | 'print' e ';'       (* print statement *)
  ;

e = LIT                 (* literal expression *)
  | ID                  (* identifier expression *)
  | '(' e ')'           (* parenthesized expression *)
  | OP1 e               (* unary expression *)
  | e OP2 e             (* binary expression *)
  ;
```
where
* `LIT` = [`0`-`9`]+
* `ID` = [`a`-`zA`-`Z`][`a`-`zA`-`Z0`-`9`]*
* `OP1` is one of: `+`, `-`
* `OP2` is one of: `*`, `+`, `-`, `/`

In addition, Let supports
* `/* C-style */` and
* `// C++-sytle` comments.

For the sake of the demo, Let is **[case-insensitive](https://en.wikipedia.org/wiki/Case_sensitivity#In_programming_languages)**:
the lexer folds every identifier and keyword to lower case (so `Foo`, `FOO`, and `foo` denote the same name, and `LET`/`Print` are recognized as keywords).
This deliberately exercises FE's case-normalizing lexer path (`fe::Lexer::accept<Append::Lower>`).

### Precedence

Ambiguities in the expression productions are resolved according to the operator precedence that is summarized in the following table (strongest binding first):

| Operator                        | Description              |
|---------------------------------|--------------------------|
| `+e`, `-e`                      | unary plus, unary minus  |
| `*`, `/`                        | multiplication, division |
| `+`, `-`                        | addition, subtraction    |

All binary operators are [**left** associative](https://en.wikipedia.org/wiki/Operator_associativity).

## Semantics

All calculations use 64-bit unsigned integer wrap-around arithmetic.
Division by zero yields zero.
Reading an identifier that was never bound by a `let` statement is **not** an error: it evaluates to zero (the name is implicitly bound to `0` on first use).
