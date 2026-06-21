# Project Knowledge

Append-only register of project-specific rules, patterns, and lessons learned.
Agents read this before every unit. Add entries when you discover something worth remembering.
## Rules

| # | Scope | Rule | Why | Added |
|---|-------|------|-----|-------|
| R1 | commit | Run `clang-format -i --style=file` on all new/modified `.cpp`/`.h` files before every commit. Use `/opt/homebrew/opt/llvm/bin/clang-format` (not in PATH). | clang-format hook has failed in 3 of 5 CI runs — the most persistent CI failure in the project. | 2026-06-21 |
| R2 | commit | Run `pre-commit run --all-files` locally before pushing when possible, or at minimum run the clang-format hook on touched C++ files. | CI code-quality gate runs pre-commit; catching issues locally avoids red builds. | 2026-06-21 |

## Patterns

| # | Pattern | Where | Notes |
|---|---------|-------|-------|

## Lessons Learned

| # | What Happened | Root Cause | Fix | Scope |
|---|--------------|------------|-----|-------|
| L1 | clang-format CI failure in 3/5 runs (actions 7, 9, 10) | Agent writes/modifies C++ files without running formatter before commit | Added R1/R2 rules; clang-format binary at `/opt/homebrew/opt/llvm/bin/clang-format` | CI, agent workflow |
