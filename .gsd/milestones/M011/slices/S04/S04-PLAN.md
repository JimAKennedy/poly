# S04: Documentation and recurrence prevention

**Goal:** Update KNOWLEDGE.md and CLAUDE.md with rules to prevent recurrence of ownership-transfer, clang-format, and NFR review issues
**Demo:** KNOWLEDGE.md and CLAUDE.md contain actionable rules for ownership-transfer, NFR review, and clang-format

## Must-Haves

- Complete the planned slice outcomes.

## Verification

- Run the task and slice verification checks for this slice.

## Tasks

- [ ] **T01: Add KNOWLEDGE.md rules for ownership-transfer and NFR review** `est:10min`
  Add new rules to KNOWLEDGE.md:
  - R6: All `new` expressions in plugin/source/ must have `// ownership-transfer` comment when ownership is transferred to VST3/VSTGUI framework (createInstance factories, createView, createCustomView, addUnit). The NFR review cpp-raw-memory scanner treats unannotated `new` as red/high.
  - R7: When adding nfr-review.yaml skip rules, always include a comment explaining WHY the rule is skipped.
  - R8: When clang-format version produces different output locally vs CI, CI is authoritative. Fix locally to match.
  - Files: `.gsd/KNOWLEDGE.md`
  - Verify: Read .gsd/KNOWLEDGE.md to confirm new rules are present and well-worded

- [ ] **T02: Update CLAUDE.md with NFR and ownership-transfer conventions** `est:10min`
  Update CLAUDE.md Key Conventions section to add:
  - A subsection on NFR Review: all `new` expressions need ownership-transfer annotation, nfr-review.yaml skip rules need rationale comments
  - Update Pre-Push Checklist to mention checking for unannotated `new` expressions when modifying plugin/source/ files
  - Note about clang-format version: CI is authoritative when local and CI disagree
  - Files: `CLAUDE.md`
  - Verify: Read CLAUDE.md to confirm new conventions are present

## Files Likely Touched

- .gsd/KNOWLEDGE.md
- CLAUDE.md
