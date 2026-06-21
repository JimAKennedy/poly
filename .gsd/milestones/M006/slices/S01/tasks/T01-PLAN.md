---
estimated_steps: 1
estimated_files: 7
skills_used: []
---

# T01: Copy harness files and CMake module

Copy visual_test_harness.h/cpp, image_compare.h/cpp, headless_ui_host.h/cpp from audio-meta/test_harness/ into tests/ui/visual/ and tests/ui/interaction/. Copy jk_ui_test_harness.cmake into cmake/. Adapt include paths and namespaces if needed.

## Inputs

- `~/dev/audio-meta/test_harness/`

## Expected Output

- `tests/ui/visual/visual_test_harness.h`
- `tests/ui/visual/image_compare.h`
- `tests/ui/interaction/headless_ui_host.h`
- `cmake/jk_ui_test_harness.cmake`

## Verification

All files exist at correct paths with no syntax errors
