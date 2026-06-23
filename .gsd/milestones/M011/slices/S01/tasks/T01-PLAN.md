---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T01: Fix clang-format disagreement in headless_ui_host.cpp

CI clang-format formats `id (*)(Class, SEL)` as `id(*)(Class, SEL)` (no space before `(`). Our local LLVM 22.1.7 preserves the space. Fix by running CI-compatible formatting on the file. Lines 144 and 148 need `id (*)` changed to `id(*)` to match CI clang-format output.

## Inputs

- `CI code-quality log showing exact diff`

## Expected Output

- `headless_ui_host.cpp with corrected function pointer cast spacing`

## Verification

/opt/homebrew/opt/llvm/bin/clang-format --dry-run --Werror tests/ui/interaction/headless_ui_host.cpp
