---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T03: Engine isolation verification

Verify that poly_engine and poly_harness compile and link without any VST3 SDK symbols. Check that no VST3 headers are included transitively. Run the harness binary to confirm it executes.

## Inputs

- `build artifacts`

## Expected Output

- `Verification evidence that engine has zero VST3 dependency`

## Verification

nm build/engine/libpoly_engine.a | grep -i steinberg returns empty; ./build/tools/harness/poly_harness exits 0
