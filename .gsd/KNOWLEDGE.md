# Project Knowledge

Append-only register of project-specific rules, patterns, and lessons learned.
Agents read this before every unit. Add entries when you discover something worth remembering.
## Rules

| # | Scope | Rule | Why | Added |
|---|-------|------|-----|-------|
| R1 | commit | Run `clang-format -i --style=file` on all new/modified `.cpp`/`.h` files before every commit. Use `/opt/homebrew/opt/llvm/bin/clang-format` (not in PATH). | clang-format hook has failed in 3 of 5 CI runs — the most persistent CI failure in the project. | 2026-06-21 |
| R2 | commit | Run `pre-commit run --all-files` locally before pushing when possible, or at minimum run the clang-format hook on touched C++ files. | CI code-quality gate runs pre-commit; catching issues locally avoids red builds. | 2026-06-21 |
| R3 | commit | Run `scripts/check-realtime-safety.sh` before committing changes to `processor.cpp` or engine files. Add `// RT-SAFE-OK` with justification for legitimate uses of flagged patterns outside the audio thread (e.g. `notify()`, `setState()`). | RT safety checker is scope-unaware — scans entire files. False positives from non-RT methods broke CI. | 2026-06-21 |
| R4 | visual-tests | Never `git add -f` visual regression baselines. They are gitignored because they are platform-specific (macOS version affects CoreGraphics rendering). Tests auto-create baselines when missing. | Baselines generated on macOS 26 (dev) produced 77% pixel diff on macOS 14 (CI runner). | 2026-06-21 |
| R5 | commit | Before pushing, verify all three local checks pass: (1) clang-format on modified C++ files, (2) `scripts/check-realtime-safety.sh`, (3) build + test suite. | CI has failed on all three categories; catching locally avoids red builds. | 2026-06-21 |

## Patterns

| # | Pattern | Where | Notes |
|---|---------|-------|-------|

## Lessons Learned

| # | What Happened | Root Cause | Fix | Scope |
|---|--------------|------------|-----|-------|
| L1 | clang-format CI failure in 3/5 runs (actions 7, 9, 10) | Agent writes/modifies C++ files without running formatter before commit | Added R1/R2 rules; clang-format binary at `/opt/homebrew/opt/llvm/bin/clang-format` | CI, agent workflow |
| L2 | RT safety check flagged `allocateMessage()`/`sendMessage()` in `notify()` | `check-realtime-safety.sh` scans entire processor.cpp without function scope awareness | Added `// RT-SAFE-OK` comments; added R3 rule to run check before commit | CI, code quality |
| L3 | 7 visual regression tests failed with 77% pixel diff | Baselines generated on macOS 26 (local), compared on macOS 14 (CI). CoreGraphics renders differently. Files were force-added despite gitignore. | Untracked baselines from git; tests auto-create on CI. Added R4 rule. | CI, visual tests |
