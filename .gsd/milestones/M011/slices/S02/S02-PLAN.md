# S02: Annotate remaining raw-memory allocations

**Goal:** Add ownership-transfer annotations to the 3 remaining unannotated new expressions in plugin code
**Demo:** NFR review shows zero red cpp-raw-memory findings

## Must-Haves

- Complete the planned slice outcomes.

## Verification

- Run the task and slice verification checks for this slice.

## Tasks

- [x] **T01: Add ownership-transfer annotation to controller.h createInstance** `est:2min`
  Line 13 of controller.h has `new PolyController()` in the static `createInstance()` factory method. This transfers ownership to the VST3 host via ref-counting (IEditController*). Add `// ownership-transfer` comment.
  - Files: `plugin/source/controller.h`
  - Verify: grep 'ownership-transfer' plugin/source/controller.h

- [x] **T02: Add ownership-transfer annotation to controller.cpp LaneEditView** `est:2min`
  Line 200 of controller.cpp has `return new LaneEditView(...)` in createCustomView() without annotation. All other view returns in the same function already have `// ownership-transfer`. This was missed in the earlier annotation pass. Add the annotation.
  - Files: `plugin/source/controller.cpp`
  - Verify: grep -n 'new LaneEditView' plugin/source/controller.cpp | grep 'ownership-transfer'

- [x] **T03: Add ownership-transfer annotation to processor.h createInstance** `est:2min`
  Line 22 of processor.h has `new PolyProcessor()` with only `// RT-SAFE-OK: host factory, not audio thread`. The NFR scanner sees the `new` but only recognizes `ownership-transfer` or `REFCOUNT-SAFE` as suppression tokens. Add `// ownership-transfer` alongside the existing RT-SAFE-OK comment.
  - Files: `plugin/source/processor.h`
  - Verify: grep 'ownership-transfer' plugin/source/processor.h

## Files Likely Touched

- plugin/source/controller.h
- plugin/source/controller.cpp
- plugin/source/processor.h
