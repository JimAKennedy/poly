---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T01: Add ownership-transfer annotation to controller.h createInstance

Line 13 of controller.h has `new PolyController()` in the static `createInstance()` factory method. This transfers ownership to the VST3 host via ref-counting (IEditController*). Add `// ownership-transfer` comment.

## Inputs

- `plugin/source/controller.h`

## Expected Output

- `controller.h with ownership-transfer annotation on createInstance new expression`

## Verification

grep 'ownership-transfer' plugin/source/controller.h
