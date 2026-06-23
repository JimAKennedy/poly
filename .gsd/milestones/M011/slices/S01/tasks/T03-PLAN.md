---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T03: Fix missing includes for MSVC cross-platform build

Add #include <algorithm> to tests/euclidean_tests.cpp:1 (needed for std::sort at line 175). Scan all test and source files for other std:: usages that rely on transitive includes which MSVC may not provide (std::sort, std::find, std::min/max without <algorithm>, std::move without <utility>, etc.).

## Inputs

- `tests/euclidean_tests.cpp`
- `all test and source .cpp files`

## Expected Output

- `tests/euclidean_tests.cpp with #include <algorithm>`

## Verification

cmake --build build && ctest --test-dir build --output-on-failure
