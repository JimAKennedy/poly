---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T03: Add ownership-transfer annotation to processor.h createInstance

Line 22 of processor.h has `new PolyProcessor()` with only `// RT-SAFE-OK: host factory, not audio thread`. The NFR scanner sees the `new` but only recognizes `ownership-transfer` or `REFCOUNT-SAFE` as suppression tokens. Add `// ownership-transfer` alongside the existing RT-SAFE-OK comment.

## Inputs

- `plugin/source/processor.h`

## Expected Output

- `processor.h with both RT-SAFE-OK and ownership-transfer annotations`

## Verification

grep 'ownership-transfer' plugin/source/processor.h
