#!/usr/bin/env bash
# run-clang-tidy-changed.behavior.test.sh
#
# M031 S04 T02 behavior proof for the shared clang-tidy gate runner
# (scripts/run-clang-tidy-changed.sh). Drives the runner through its explicit
# file-list path with two synthetic inputs and asserts the gate is NOT a
# rubber stamp:
#
#   1. GREEN — a clean C++ file with none of the gated WarningsAsErrors defects
#      exits 0 and prints the PASS line.
#   2. RED   — a file introducing a `modernize-use-nullptr` defect (`int* p = 0;`)
#      exits 1 and prints the offending `file:line:col ... [modernize-use-nullptr` so
#      a contributor sees exactly which gated check fired.
#
# modernize-use-nullptr is one of the 8 checks in the `.clang-tidy`
# WarningsAsErrors set, so proving it fires proves the gate enforces that set.
#
# Skips (exit 77, the automake skip convention) when no clang-tidy binary is
# resolvable, so the suite stays green on machines without LLVM installed. CI
# installs clang-tidy, so the gate is exercised for real there.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
RUNNER="$REPO_ROOT/scripts/run-clang-tidy-changed.sh"

fail() {
    echo "behavior test FAIL: $1" >&2
    exit 1
}

[ -x "$RUNNER" ] || fail "runner not found or not executable at $RUNNER"

# --- Resolve clang-tidy the same way the runner does; skip if absent ---------
if [ -n "${POLY_TIDY_BIN:-}" ]; then
    TIDY="$POLY_TIDY_BIN"
elif command -v clang-tidy >/dev/null 2>&1; then
    TIDY="$(command -v clang-tidy)"
elif [ -x /opt/homebrew/opt/llvm/bin/clang-tidy ]; then
    TIDY="/opt/homebrew/opt/llvm/bin/clang-tidy"
else
    echo "SKIP: clang-tidy not found (no POLY_TIDY_BIN, not on PATH, no Homebrew LLVM)"
    exit 77
fi
export POLY_TIDY_BIN="$TIDY"

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

# --- Case 1: clean file → exit 0 ---------------------------------------------
CLEAN="$WORK/clean.cpp"
cat >"$CLEAN" <<'EOF'
// No gated WarningsAsErrors defect here.
int add(int a, int b) {
    int* p = nullptr;
    (void)p;
    return a + b;
}
EOF

set +e
CLEAN_OUT="$("$RUNNER" "$CLEAN" 2>&1)"
CLEAN_RC=$?
set -e
echo "$CLEAN_OUT"
[ "$CLEAN_RC" -eq 0 ] || fail "clean file expected exit 0, got $CLEAN_RC"
echo "$CLEAN_OUT" | grep -q "PASS" || fail "clean file did not print PASS line"
echo "behavior test: GREEN case passed (clean file → exit 0)"

# --- Case 2: modernize-use-nullptr defect → exit 1 ---------------------------
DEFECT="$WORK/defect.cpp"
cat >"$DEFECT" <<'EOF'
// `int* p = 0;` is a modernize-use-nullptr defect (a gated WarningsAsErrors check).
int use() {
    int* p = 0;
    return p == nullptr ? 1 : 0;
}
EOF

set +e
DEFECT_OUT="$("$RUNNER" "$DEFECT" 2>&1)"
DEFECT_RC=$?
set -e
echo "$DEFECT_OUT"
[ "$DEFECT_RC" -eq 1 ] || fail "defect file expected exit 1, got $DEFECT_RC"
echo "$DEFECT_OUT" | grep -q "modernize-use-nullptr" \
    || fail "defect file did not name the modernize-use-nullptr check in its output"
echo "$DEFECT_OUT" | grep -qE "defect\.cpp:[0-9]+:[0-9]+" \
    || fail "defect file did not print an offending file:line:col location"
echo "behavior test: RED case passed (nullptr defect → exit 1 with file:line:check)"

echo "behavior test: ALL PASSED"
