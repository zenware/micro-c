#!/usr/bin/env bash
set -Eeuo pipefail

SOURCE="microcc.c"
COMPILER="./microcc"
ASSEMBLY="generated.s"
EXECUTABLE="./generated"

cleanup() {
    rm -f "$ASSEMBLY" "$EXECUTABLE"
}

trap cleanup EXIT

fail() {
    echo "ERROR: $*" >&2
    exit 1
}

# Compile the C program.
echo "Compiling $SOURCE..."
cc -Wall -Wextra -Wpedantic -o "$COMPILER" "$SOURCE" \
    || fail "Failed to compile $SOURCE"

# Test a generated program against an expected exit code.
assert() {
    local expected="$1"
    local input="$2"
    local actual

    echo "$COMPILER $input"

    "$COMPILER" "$input" > "$ASSEMBLY" \
        || fail "Code generation failed for input: $input"

    cc -o "$EXECUTABLE" "$ASSEMBLY" \
        || fail "Assembly failed for input: $input"

    if "$EXECUTABLE"; then
        actual=0
    else
        actual=$?
    fi

    if [[ "$actual" -ne "$expected" ]]; then
        fail "$input => expected $expected, got $actual"
    fi

    echo "PASS: $input => $actual"
}

assert 0 0
assert 42 42
assert 21 "5+20-4"
assert 41 " 12 + 34 - 5 "
assert 47 '5+6*7'
assert 15 '5*(9-6)'
assert 4 '(3+5)/2'
assert 30 "10 + 20"
assert 90 "(10+20)*3"
assert 10 "100 /  5  / 2"
assert 50 "100 -20 -30"


echo "All tests passed!"
