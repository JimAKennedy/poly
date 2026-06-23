---
estimated_steps: 4
estimated_files: 1
skills_used: []
---

# T01: Add KNOWLEDGE.md rules for ownership-transfer and NFR review

Add new rules to KNOWLEDGE.md:
- R6: All `new` expressions in plugin/source/ must have `// ownership-transfer` comment when ownership is transferred to VST3/VSTGUI framework (createInstance factories, createView, createCustomView, addUnit). The NFR review cpp-raw-memory scanner treats unannotated `new` as red/high.
- R7: When adding nfr-review.yaml skip rules, always include a comment explaining WHY the rule is skipped.
- R8: When clang-format version produces different output locally vs CI, CI is authoritative. Fix locally to match.

## Inputs

- `.gsd/KNOWLEDGE.md`

## Expected Output

- `.gsd/KNOWLEDGE.md with R6, R7, R8 rules`

## Verification

Read .gsd/KNOWLEDGE.md to confirm new rules are present and well-worded
