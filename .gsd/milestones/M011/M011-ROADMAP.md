# M011: CI and Code Quality Green Baseline

**Vision:** Eliminate all red and amber CI findings so PRs pass code-quality and nfr-review gates cleanly. Update project documentation to prevent recurrences of ownership-transfer annotation gaps, clang-format version mismatches, and NFR false positives.

## Success Criteria

- All CI checks pass on a test PR (code-quality, nfr-review, builds, engine-isolation, secrets-scan, coverage)
- Zero red NFR findings on main branch
- Zero amber NFR findings on main branch (via fix or justified skip)
- KNOWLEDGE.md and CLAUDE.md updated with ownership-transfer and NFR conventions
- clang-format version pinned to prevent future CI/local mismatches

## Slices

- [x] **S01: S01** `risk:low` `depends:[]`
  > After this: pre-commit run --all-files passes locally and in CI with identical formatting

- [x] **S02: S02** `risk:low` `depends:[]`
  > After this: NFR review shows zero red cpp-raw-memory findings

- [x] **S03: S03** `risk:medium` `depends:[]`
  > After this: NFR review shows zero red and zero amber findings

- [ ] **S04: S04** `risk:low` `depends:[]`
  > After this: KNOWLEDGE.md and CLAUDE.md contain actionable rules for ownership-transfer, NFR review, and clang-format

## Boundary Map

Not provided.
