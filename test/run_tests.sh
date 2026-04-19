#!/usr/bin/env bash
set -uo pipefail

LET=${1:-./build/bin/let}
PASS=0
FAIL=0
TOTAL=0

stdout_tmp=$(mktemp)
stderr_tmp=$(mktemp)
trap 'rm -f "$stdout_tmp" "$stderr_tmp"' EXIT

red()   { printf '\033[1;31m%s\033[0m\n' "$*"; }
green() { printf '\033[1;32m%s\033[0m\n' "$*"; }

for letf in test/*.let; do
    name=${letf%.let}
    base=$(basename "$name")
    ((TOTAL++))

    if [[ ! -f "$name.out" ]]; then
        "$LET" "$letf" -e > "$name.out" 2>/dev/null
        green "GENERATED: $name.out"
        ((TOTAL--))
        continue
    fi

    # Eval test: run with -e, expect success, compare stdout
    "$LET" "$letf" -e > "$stdout_tmp" 2> "$stderr_tmp"
    rc=$?
    if [[ $rc -ne 0 ]]; then
        red "FAIL: $base (exit code $rc)"
        sed 's/^/  /' "$stderr_tmp"
        ((FAIL++))
        continue
    fi
    if diff -u --label expected --label actual "$name.out" "$stdout_tmp" > /dev/null 2>&1; then
        green "PASS: $base"
        ((PASS++))
    else
        red "FAIL: $base (output mismatch)"
        diff -u --label expected --label actual "$name.out" "$stdout_tmp" | sed 's/^/  /'
        ((FAIL++))
    fi
done

for letf in test/error/*.let; do
    [[ -e "$letf" ]] || continue
    name=${letf%.let}
    base=$(basename "$name")
    ((TOTAL++))

    if [[ ! -f "$name.err" ]]; then
        "$LET" "$letf" 2> "$name.err" || true
        green "GENERATED: $name.err"
        ((TOTAL--))
        continue
    fi

    # Error test: expect non-zero exit, check patterns in stderr
    "$LET" "$letf" > "$stdout_tmp" 2> "$stderr_tmp"
    rc=$?
    if [[ $rc -eq 0 ]]; then
        red "FAIL: $base (expected failure but exited 0)"
        ((FAIL++))
        continue
    fi
    ok=true
    while IFS= read -r pattern; do
        pattern="${pattern%$'\r'}"
        [[ -z "$pattern" || "$pattern" == \#* ]] && continue
        if ! grep -qF "$pattern" "$stderr_tmp"; then
            red "FAIL: $base (missing pattern: $pattern)"
            echo "  stderr was:"
            sed 's/^/    /' "$stderr_tmp"
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
