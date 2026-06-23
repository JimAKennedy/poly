#!/usr/bin/env bash
set -euo pipefail

# Pre-push hook: block direct pushes to main, enforce quality checks.
# Install via: pre-commit install -t pre-push

PROTECTED_BRANCH="main"
FAILED=0

while read -r local_ref local_sha remote_ref remote_sha; do
    remote_branch="${remote_ref#refs/heads/}"
    if [ "$remote_branch" = "$PROTECTED_BRANCH" ]; then
        echo "ERROR: Direct push to '$PROTECTED_BRANCH' is blocked."
        echo "  Create a feature branch and open a PR instead:"
        echo "    git checkout -b fix/my-change"
        echo "    git push -u origin fix/my-change"
        echo "    gh pr create"
        echo "  (bypass with --no-verify for emergencies)"
        exit 1
    fi
done

echo "=== Pre-push quality checks ==="

echo "[1/3] clang-format..."
if ! pre-commit run clang-format --all-files; then
    echo "FAIL: clang-format found formatting issues."
    FAILED=1
fi

echo "[2/3] RT safety..."
if ! scripts/check-realtime-safety.sh; then
    echo "FAIL: RT safety check failed."
    FAILED=1
fi

echo "[3/3] Build + test..."
if ! cmake --build build --config Release --parallel 2>/dev/null; then
    echo "FAIL: Build failed."
    FAILED=1
elif ! ctest --test-dir build --build-config Release --output-on-failure 2>/dev/null; then
    echo "FAIL: Tests failed."
    FAILED=1
fi

if [ "$FAILED" -ne 0 ]; then
    echo "=== Pre-push checks FAILED ==="
    exit 1
fi

echo "=== Pre-push checks passed ==="
