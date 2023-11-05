# Let

[![linux](https://img.shields.io/github/actions/workflow/status/leissa/let/linux.yml?logo=linux&logoColor=white&label=linux&link=https%3A%2F%2Fgithub.com%2Fleissa%2Flet%2Factions%2Fworkflows%2Flinux.yml)](https://github.com/leissa/let/actions/workflows/linux.yml)
[![macos](https://img.shields.io/github/actions/workflow/status/leissa/let/macos.yml?logo=apple&logoColor=white&label=macos&link=https%3A%2F%2Fgithub.com%2Fleissa%2Flet%2Factions%2Fworkflows%2Fmacos.yml)](https://github.com/leissa/let/actions/workflows/macos.yml)
[![windows](https://img.shields.io/github/actions/workflow/status/leissa/let/windows.yml?logo=windows&logoColor=white&label=windows&link=https%3A%2F%2Fgithub.com%2Flet%2Fleissa%2Factions%2Fworkflows%2Fwindows.yml)](https://github.com/leissa/let/actions/workflows/windows.yml)

A simple demo language that builds upon [FE](https://leissa.github.io/fe/).

## Usage

```
USAGE:
  let [-?|-h|--help] [-v|--version] [-d|--dump] [-e|--eval] [<file>]

Display usage information.

OPTIONS, ARGUMENTS:
  -?, -h, --help
  -v, --version           Display version info and exit.
  -d, --dump              Dumps the let program again.
  -e, --eval              Evaluate the let program.
  <file>                  Input file.

Use "-" as <file> to output to stdout.
```

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

For the sake of the demo, Let is **[case-insensitive](https://en.wikipedia.org/wiki/Case_sensitivity#In_programming_languages)**.

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
