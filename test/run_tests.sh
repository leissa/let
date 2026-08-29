#!/usr/bin/env bash
set -uo pipefail

LET=${1:-./build/bin/let}
PASS=0
FAIL=0
TOTAL=0

stdout_tmp=$(mktemp)
stderr_tmp=$(mktemp)
actual_tmp=$(mktemp)
trap 'rm -f "$stdout_tmp" "$stderr_tmp" "$actual_tmp"' EXIT

# On Windows the C runtime opens stdout/stderr in text mode, so the binary emits
# CRLF while the goldens are checked out with LF (see .gitattributes). Strip the
# CRs from what the binary produced before comparing or generating goldens.
# LC_ALL=C makes tr operate byte-wise: the error tests feed the binary invalid
# UTF-8, and BSD tr (macOS) bails out with "Illegal byte sequence" otherwise.
strip_cr() { LC_ALL=C tr -d '\r' < "$1" > "$2"; }

red()   { printf '\033[1;31m%s\033[0m\n' "$*"; }
green() { printf '\033[1;32m%s\033[0m\n' "$*"; }

for letf in test/*.let; do
    name=${letf%.let}
    base=$(basename "$name")
    ((TOTAL++))

    if [[ ! -f "$name.out" ]]; then
        "$LET" "$letf" -e 2>/dev/null | LC_ALL=C tr -d '\r' > "$name.out"
        green "GENERATED: $name.out"
        ((TOTAL--))
        continue
    fi

    # Eval test: run with -e, expect success, compare stdout
    "$LET" "$letf" -e > "$stdout_tmp" 2> "$stderr_tmp"
    rc=$?
    strip_cr "$stdout_tmp" "$actual_tmp"
    if [[ $rc -ne 0 ]]; then
        red "FAIL: $base (exit code $rc)"
        sed 's/^/  /' "$stderr_tmp"
        ((FAIL++))
        continue
    fi
    if diff -u --label expected --label actual "$name.out" "$actual_tmp" > /dev/null 2>&1; then
        green "PASS: $base"
        ((PASS++))
    else
        red "FAIL: $base (output mismatch)"
        diff -u --label expected --label actual "$name.out" "$actual_tmp" | sed 's/^/  /'
        ((FAIL++))
    fi
done

for letf in test/error/*.let; do
    [[ -e "$letf" ]] || continue
    name=${letf%.let}
    base=$(basename "$name")
    ((TOTAL++))

    # An optional <name>.flags file holds extra CLI arguments for this test.
    flags=()
    [[ -f "$name.flags" ]] && read -r -a flags < "$name.flags"

    if [[ ! -f "$name.err" ]]; then
        "$LET" "$letf" "${flags[@]}" 2>&1 >/dev/null | LC_ALL=C tr -d '\r' > "$name.err"
        green "GENERATED: $name.err"
        ((TOTAL--))
        continue
    fi

    # Error test: expect non-zero exit, check patterns in stderr
    "$LET" "$letf" "${flags[@]}" > "$stdout_tmp" 2> "$stderr_tmp"
    rc=$?
    strip_cr "$stderr_tmp" "$actual_tmp"
    if [[ $rc -eq 0 ]]; then
        red "FAIL: $base (expected failure but exited 0)"
        ((FAIL++))
        continue
    fi
    ok=true
    while IFS= read -r pattern; do
        pattern="${pattern%$'\r'}"
        [[ -z "$pattern" || "$pattern" == \#* ]] && continue
        if ! LC_ALL=C grep -qF "$pattern" "$actual_tmp"; then
            red "FAIL: $base (missing pattern: $pattern)"
            echo "  stderr was:"
            sed 's/^/    /' "$actual_tmp"
            ok=false
            break
        fi
    done < "$name.err"
    if $ok; then
        green "PASS: $base"
        ((PASS++))
    else
        ((FAIL++))
    fi
done

echo
echo "$PASS/$TOTAL passed, $FAIL failed"
[[ $FAIL -eq 0 ]]
