---
estimated_steps: 5
estimated_files: 1
skills_used: []
---

# T03: Skip or fix cmake-build-config amber findings

8 amber/low cmake-build-config findings for subdirectory CMakeLists missing project() VERSION and target_compile_features. These subdirectories (engine, plugin, tools/harness) inherit from the root CMakeLists.txt which sets the C++ standard via CMAKE_CXX_STANDARD 20.

Options:
1. Add the rule to nfr-review.yaml skip list with rationale (subdirectories inherit from root)
2. Add target_compile_features to each subdirectory

Recommendation: Skip in nfr-review.yaml since subdirectory CMakeLists inheriting standards from root is standard CMake practice, and adding project(VERSION) to subdirectories can conflict with the root project version.

## Inputs

- `nfr-review.yaml`
- `engine/CMakeLists.txt`
- `plugin/CMakeLists.txt`
- `tools/harness/CMakeLists.txt`

## Expected Output

- `nfr-review.yaml with cmake-build-config in skip list, or subdirectory CMakeLists with compile features`

## Verification

Read nfr-review.yaml to confirm rule is in skip list with comment
