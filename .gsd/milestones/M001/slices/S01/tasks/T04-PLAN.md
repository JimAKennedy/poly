---
estimated_steps: 5
estimated_files: 1
skills_used: []
---

# T04: Build verification and warning cleanup

1. Run full cmake configure + build from clean state
2. Verify zero warnings with -Wall -Wextra
3. Verify .vst3 bundle exists in build output
4. Verify poly_engine compiles independently (no VST3 SDK leakage)
5. Update .gitignore if needed for SDK artifacts

## Inputs

- `CMakeLists.txt`
- `engine/CMakeLists.txt`
- `plugin/CMakeLists.txt`

## Expected Output

- `Clean build log`
- `.vst3 bundle path`

## Verification

cmake --build build --clean-first 2>&1 | grep -c 'warning:' returns 0; find build -name '*.vst3' returns a path
