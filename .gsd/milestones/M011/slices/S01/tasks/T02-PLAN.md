---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T02: Reformat all C++ files with pinned clang-format version

After pinning the version, run clang-format on ALL .cpp/.h files to normalize formatting. The id(*) vs id (*) spacing in headless_ui_host.cpp:144,148 will be resolved by this pass. This is a one-time bulk reformat to establish the baseline with the pinned version.

## Inputs

- `all .cpp and .h files`

## Expected Output

- `all C++ files formatted consistently`

## Verification

pre-commit run clang-format --all-files exits 0
