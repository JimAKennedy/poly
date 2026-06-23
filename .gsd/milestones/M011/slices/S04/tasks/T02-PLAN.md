---
estimated_steps: 4
estimated_files: 1
skills_used: []
---

# T02: Update CLAUDE.md with NFR and ownership-transfer conventions

Update CLAUDE.md Key Conventions section to add:
- A subsection on NFR Review: all `new` expressions need ownership-transfer annotation, nfr-review.yaml skip rules need rationale comments
- Update Pre-Push Checklist to mention checking for unannotated `new` expressions when modifying plugin/source/ files
- Note about clang-format version: CI is authoritative when local and CI disagree

## Inputs

- `CLAUDE.md`

## Expected Output

- `CLAUDE.md with NFR Review conventions and updated pre-push checklist`

## Verification

Read CLAUDE.md to confirm new conventions are present
