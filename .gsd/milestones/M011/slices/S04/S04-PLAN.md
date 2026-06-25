# S04: Documentation and recurrence prevention

**Goal:** Add local push quality gates (pre-push hook) since GitHub branch protection requires Pro for private repos, and update documentation to prevent recurrence of all issues found in this milestone
**Demo:** KNOWLEDGE.md and CLAUDE.md contain actionable rules for ownership-transfer, NFR review, and clang-format

## Must-Haves

- pre-push hook blocks pushes that fail code-quality or build; KNOWLEDGE.md and CLAUDE.md updated with ownership-transfer, clang-format, and MSVC portability rules

## Verification

- Run the task and slice verification checks for this slice.

## Tasks

- [x] **T01: Add pre-push Git hook via pre-commit framework** `est:20min`
  Add a pre-push stage to .pre-commit-config.yaml that runs: (1) clang-format on modified C++ files, (2) check-realtime-safety.sh, (3) cmake build + ctest. This enforces the pre-push checklist automatically. Since GitHub branch protection requires Pro for private repos, this is our local enforcement mechanism. Document the limitation and suggest upgrading to Pro or making the repo public when branch protection becomes needed.
  - Files: `.pre-commit-config.yaml`, `scripts/pre-push-check.sh`
  - Verify: git push to a test branch triggers the pre-push hook and blocks on failure

- [x] **T02: Add KNOWLEDGE.md rules for all discovered issues** `est:10min`
  Add rules to KNOWLEDGE.md: R6: All new expressions in plugin/source/ must have // ownership-transfer when ownership transfers to VST3/VSTGUI. R7: Always include <algorithm> and other standard headers explicitly; MSVC does not provide them transitively. R8: clang-format version is pinned in .pre-commit-config.yaml; do not change without verifying CI compatibility. R9: nfr-review.yaml skip rules must have rationale comments.
  - Files: `.gsd/KNOWLEDGE.md`
  - Verify: grep -c 'R[6-9]' .gsd/KNOWLEDGE.md shows 4 new rules

- [x] **T03: Update CLAUDE.md with NFR, ownership-transfer, and MSVC conventions** `est:10min`
  Update CLAUDE.md Key Conventions: (1) Add NFR Review subsection covering ownership-transfer annotation requirement; (2) Add MSVC Portability subsection about explicit includes; (3) Update Pre-Push Checklist to note it is now automated via pre-push hook; (4) Note clang-format is pinned and CI-authoritative.
  - Files: `CLAUDE.md`
  - Verify: Read CLAUDE.md to confirm new conventions present

- [x] **T04: Final end-to-end verification push** `est:15min`
  Push all changes to main, verify CI passes all checks (code-quality, all platform builds, engine-isolation, secrets-scan, coverage). If any check fails, fix and re-push. This is the acceptance test for the entire milestone.
  - Verify: gh run list --limit 1 shows CI passing on main

## Files Likely Touched

- .pre-commit-config.yaml
- scripts/pre-push-check.sh
- .gsd/KNOWLEDGE.md
- CLAUDE.md
