---
estimated_steps: 6
estimated_files: 1
skills_used: []
---

# T01: Top-level CMake with VST3 SDK via FetchContent

Create the top-level CMakeLists.txt that:
1. Sets project(poly) with C++20 standard
2. Fetches the Steinberg VST3 SDK via FetchContent from GitHub
3. Includes cmake/jk_warnings.cmake
4. Adds engine/ and plugin/ subdirectories
5. Adds VST3 SDK build output paths

## Inputs

- `cmake/jk_warnings.cmake`
- `IMPLEMENTATION_PLAN.md section 2`

## Expected Output

- `CMakeLists.txt`

## Verification

cmake -B build -DCMAKE_BUILD_TYPE=Debug completes without error
