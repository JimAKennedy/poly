#!/usr/bin/env bash
# run-clang-tidy-changed.sh
#
# Shared clang-tidy gate runner. Runs clang-tidy on PR-changed engine C/C++
# files and fails ONLY on the checks listed in the repo `.clang-tidy`
# WarningsAsErrors set (8 real-defect checks). The ~1,400 legacy full-roster
# warnings the engine tree still carries stay advisory and are NOT run here — the
# gate is deliberately scoped to the WarningsAsErrors set so it is green on the
# current tree (D032 recon: 0 WarningsAsErrors hits engine-wide today) while
# still catching a newly-introduced use-after-move, dangling-handle, raw
# nullptr, uninitialised member, etc.
#
# The WarningsAsErrors list is parsed FROM `.clang-tidy` at runtime, so that
# config file stays the single source of truth — editing the gated set there
# changes what this runner enforces, with no second list to keep in sync.
#
# Usage:
#   scripts/run-clang-tidy-changed.sh [file ...]
#     With no args, computes changed files via `git diff` against POLY_TIDY_BASE
#     (default: HEAD^). With explicit file args, gates exactly those files
#     (used by the behavior test to drive a clean file and a defect file).
#
# Environment:
#   POLY_TIDY_BIN        clang-tidy binary (auto-detected if unset:
#                        `clang-tidy` on PATH, else the Homebrew LLVM path).
#   POLY_TIDY_BUILD_DIR  directory holding compile_commands.json (default:
#                        build-tidy). If present, passed via `-p`; if absent, a
#                        fixed `-- -std=c++20 -I engine/include` command is used
#                        so a standalone file can still be analyzed.
#   POLY_TIDY_BASE       git ref/SHA to diff against for changed files
#                        (default: HEAD^). Ignored when file args are given.
#
# Exit codes: 0 = gate green (no gated defect, or no engine files changed);
#             1 = at least one gated WarningsAsErrors check fired (file:line:check
#                 printed by clang-tidy above the verdict); 2 = setup error
#                 (clang-tidy missing, .clang-tidy missing).

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
CLANG_TIDY_CONFIG="$REPO_ROOT/.clang-tidy"

# region:clang-tidy-runner
# --- Resolve clang-tidy binary ---------------------------------------------
resolve_clang_tidy() {
    if [ -n "${POLY_TIDY_BIN:-}" ]; then
        echo "$POLY_TIDY_BIN"
        return 0
    fi
    if command -v clang-tidy >/dev/null 2>&1; then
        command -v clang-tidy
        return 0
    fi
    # macOS Homebrew LLVM is not on PATH by default (see CLAUDE.md).
    if [ -x /opt/homebrew/opt/llvm/bin/clang-tidy ]; then
        echo /opt/homebrew/opt/llvm/bin/clang-tidy
        return 0
    fi
    return 1
}

CLANG_TIDY_BIN="$(resolve_clang_tidy)" || {
    echo "run-clang-tidy-changed.sh: clang-tidy not found (set POLY_TIDY_BIN)" >&2
    exit 2
}

if [ ! -f "$CLANG_TIDY_CONFIG" ]; then
    echo "run-clang-tidy-changed.sh: .clang-tidy config not found at $CLANG_TIDY_CONFIG" >&2
    exit 2
fi

# --- Parse the WarningsAsErrors gated set from .clang-tidy ------------------
# Grab every line after the `WarningsAsErrors:` key up to the next top-level
# YAML key, then strip whitespace/newlines so the folded (`>`) scalar becomes a
# single comma-separated check list. The last entry carries no trailing comma,
# so the collapsed result is a clean `a,b,c` list.
GATED_CHECKS="$(
    awk '
        /^WarningsAsErrors:/ { grab = 1; next }
        grab && /^[A-Za-z][A-Za-z0-9_]*:/ { grab = 0 }
        grab { print }
    ' "$CLANG_TIDY_CONFIG" | tr -d '[:space:]'
)"

if [ -z "$GATED_CHECKS" ]; then
    echo "run-clang-tidy-changed.sh: could not parse WarningsAsErrors from .clang-tidy" >&2
    exit 2
fi

# --- Determine the changed engine files to gate ----------------------------
# C/C++ source and header extensions under engine/. Headers may lack a compile
# command; clang-tidy skips those with a note rather than failing the gate.
is_engine_cxx() {
    case "$1" in
        engine/*.cpp | engine/*.cc | engine/*.cxx | engine/*.c | engine/*.h | engine/*.hpp) return 0 ;;
        *) return 1 ;;
    esac
}

CHANGED_FILES=()
if [ "$#" -gt 0 ]; then
    # Explicit file list (behavior test path). Take the args verbatim — the
    # caller is responsible for what it hands us; this lets a synthetic defect
    # file outside engine/ be gated in a test.
    for f in "$@"; do
        CHANGED_FILES+=("$f")
    done
else
    BASE_REF="${POLY_TIDY_BASE:-HEAD^}"
    if ! git -C "$REPO_ROOT" rev-parse --verify --quiet "$BASE_REF" >/dev/null; then
        echo "run-clang-tidy-changed.sh: base ref '$BASE_REF' not found — skipping (no diff base)"
        exit 0
    fi
    while IFS= read -r f; do
        [ -n "$f" ] || continue
        if is_engine_cxx "$f" && [ -f "$REPO_ROOT/$f" ]; then
            CHANGED_FILES+=("$f")
        fi
    done < <(git -C "$REPO_ROOT" diff --name-only --diff-filter=ACMRT "$BASE_REF"...HEAD)
fi

if [ "${#CHANGED_FILES[@]}" -eq 0 ]; then
    echo "run-clang-tidy-changed.sh: no changed engine C/C++ files — skipping clang-tidy gate"
    exit 0
fi

# --- Build the clang-tidy invocation ---------------------------------------
# Restrict to exactly the gated checks and promote them to errors, so the
# process exit code reflects ONLY the WarningsAsErrors set. Advisory
# full-roster checks are not run here (kept green + fast + focused log).
TIDY_ARGS=("--quiet" "--checks=-*,$GATED_CHECKS" "--warnings-as-errors=$GATED_CHECKS")

BUILD_DIR="${POLY_TIDY_BUILD_DIR:-build-tidy}"
COMPILE_DB_MODE=""
POST_ARGS=()
if [ -f "$REPO_ROOT/$BUILD_DIR/compile_commands.json" ]; then
    TIDY_ARGS+=("-p" "$REPO_ROOT/$BUILD_DIR")
    COMPILE_DB_MODE="compile_commands.json ($BUILD_DIR)"
else
    # No compilation database: give clang-tidy a fixed command so a standalone
    # file still analyzes. Engine is C++20 with public headers in engine/include.
    POST_ARGS=("--" "-std=c++20" "-I" "$REPO_ROOT/engine/include")
    COMPILE_DB_MODE="fixed flags (-std=c++20 -I engine/include; no compile DB)"
fi

echo "run-clang-tidy-changed.sh: gating ${#CHANGED_FILES[@]} changed engine file(s) with clang-tidy"
echo "  binary : $CLANG_TIDY_BIN"
echo "  compile: $COMPILE_DB_MODE"
echo "  gated  : $GATED_CHECKS"
for f in "${CHANGED_FILES[@]}"; do
    echo "    - $f"
done

# --- Run the gate ----------------------------------------------------------
RC=0
for f in "${CHANGED_FILES[@]}"; do
    if ! "$CLANG_TIDY_BIN" "${TIDY_ARGS[@]}" "$f" "${POST_ARGS[@]}"; then
        RC=1
    fi
done

if [ "$RC" -ne 0 ]; then
    echo "run-clang-tidy-changed.sh: FAIL — a gated WarningsAsErrors check fired (see file:line:check above)" >&2
    exit 1
fi

echo "run-clang-tidy-changed.sh: PASS — 0 WarningsAsErrors defects in changed engine files"
# endregion:clang-tidy-runner
