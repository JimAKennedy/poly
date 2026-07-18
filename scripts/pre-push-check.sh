#!/usr/bin/env bash
set -euo pipefail

# Pre-push hook: block direct pushes to main, enforce quality checks.
# Install via: pre-commit install -t pre-push
#
# Install pluginval to enable the CI-parity step 5:
#   bash scripts/install-pluginval.sh
#
# Environment overrides:
#   PLUGINVAL_STRICTNESS=8   # default 5 locally; CI uses 8. Bump to 8 for CI-parity check.

PROTECTED_BRANCH="main"
FAILED=0
PLUGINVAL_STRICTNESS="${PLUGINVAL_STRICTNESS:-5}"

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

# region:pre-push-gates
echo "[0/5] Build config check..."
CACHE_FILE="build/CMakeCache.txt"
NEEDS_RECONFIG=0
RECONFIG_REASONS=()
if [ -f "$CACHE_FILE" ]; then
    if grep -q '^POLY_ENGINE_ONLY:BOOL=ON' "$CACHE_FILE"; then
        NEEDS_RECONFIG=1
        RECONFIG_REASONS+=("POLY_ENGINE_ONLY=ON (host tests would be skipped)")
    fi
    # BUILD_INTERACTION_TESTS gates poly_interaction_tests (and by extension the
    # POLY_PLUGIN_SOURCES link check). If it's OFF, a missing source in that list
    # only trips on CI — see the T01 fixup (controller_base.cpp) that shipped
    # a red build to CI because local hooks compiled without this target.
    if ! grep -q '^BUILD_INTERACTION_TESTS:BOOL=ON' "$CACHE_FILE"; then
        NEEDS_RECONFIG=1
        RECONFIG_REASONS+=("BUILD_INTERACTION_TESTS!=ON (CI-parity link check would be skipped)")
    fi
fi
if [ "$NEEDS_RECONFIG" -eq 1 ]; then
    for reason in "${RECONFIG_REASONS[@]}"; do
        echo "  build/ needs reconfigure: $reason"
    done
    if ! cmake -S . -B build -DPOLY_ENGINE_ONLY=OFF -DBUILD_INTERACTION_TESTS=ON >/dev/null; then
        echo "FAIL: could not reconfigure build/ with POLY_ENGINE_ONLY=OFF -DBUILD_INTERACTION_TESTS=ON."
        FAILED=1
    fi
fi

echo "[1/5] clang-format..."
# Prefer running on staged files (fast, no full-tree traversal) when we have any;
# fall back to --all-files if git diff --cached is empty (e.g. hook invoked
# outside a staged context, or the whole tree just got reformatted).
STAGED_CPP=$(git diff --cached --name-only --diff-filter=ACMR 2>/dev/null | grep -E '\.(cpp|h|hpp|cc)$' || true)
if [ -n "$STAGED_CPP" ]; then
    if ! pre-commit run clang-format --files $STAGED_CPP; then
        echo "FAIL: clang-format found formatting issues (staged files)."
        FAILED=1
    fi
else
    if ! pre-commit run clang-format --all-files; then
        echo "FAIL: clang-format found formatting issues (full tree)."
        FAILED=1
    fi
fi

echo "[2/5] RT safety..."
if ! scripts/check-realtime-safety.sh; then
    echo "FAIL: RT safety check failed."
    FAILED=1
fi

echo "[3/5] CodeSnippet region markers..."
if ! scripts/check-snippet-regions.sh; then
    echo "FAIL: CodeSnippet region check failed."
    FAILED=1
fi

echo "[4/5] Build + test..."
if ! cmake --build build --config Release --parallel 2>/dev/null; then
    echo "FAIL: Build failed."
    FAILED=1
elif ! ctest --test-dir build --build-config Release --output-on-failure 2>/dev/null; then
    echo "FAIL: Tests failed."
    FAILED=1
fi

echo "[5/5] pluginval (strictness=${PLUGINVAL_STRICTNESS})..."
if ! command -v pluginval >/dev/null 2>&1; then
    echo "  SKIP: pluginval not on PATH. Install with: bash scripts/install-pluginval.sh"
else
    VST3_PATH=$(find build -type d -name '*.vst3' -not -name '*probe*' 2>/dev/null | head -1)
    if [ -z "$VST3_PATH" ]; then
        echo "  SKIP: no .vst3 bundle in build/ (engine-only build produces none)."
    else
        echo "  Validating $VST3_PATH..."
        if ! pluginval --strictness-level "$PLUGINVAL_STRICTNESS" --skip-gui-tests --timeout-ms 60000 \
             --validate "$VST3_PATH"; then
            echo "FAIL: pluginval reported errors at strictness $PLUGINVAL_STRICTNESS."
            echo "  For CI-parity strictness: PLUGINVAL_STRICTNESS=8 bash scripts/pre-push-check.sh"
            FAILED=1
        fi
    fi
fi
# endregion:pre-push-gates

if [ "$FAILED" -ne 0 ]; then
    echo "=== Pre-push checks FAILED ==="
    exit 1
fi

echo "=== Pre-push checks passed ==="
