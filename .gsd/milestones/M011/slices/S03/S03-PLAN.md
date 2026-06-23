# S03: Resolve remaining NFR red and amber findings

**Goal:** Resolve all remaining NFR red and amber findings to reach zero findings on the PR gate. Address each finding category with a fix or a justified skip.
**Demo:** NFR review shows zero red and zero amber findings

## Must-Haves

- NFR review run locally produces zero red findings; all amber findings either fixed or skipped with rationale in nfr-review.yaml

## Verification

- Run the task and slice verification checks for this slice.

## Tasks

- [x] **T01: Skip cmake-minimum-version for subdirectory CMakeLists (already in skip list, verify)** `est:5min`
  cmake-minimum-version is already in nfr-review.yaml skip list but 4 red findings appeared on PR. Verify the skip is working correctly on current main. If the skip wasn't in the PR branch's merge base, this is already fixed by the earlier merge. Confirm by checking the nfr-review.yaml on main.
  - Files: `nfr-review.yaml`
  - Verify: grep cmake-minimum-version nfr-review.yaml

- [x] **T02: Pin VST3 SDK FetchContent to commit hash** `est:10min`
  cmake-fetchcontent-pinning (amber) flags VST3 SDK using tag v3.7.12_build_20 instead of a commit hash. Resolve the tag to its commit SHA on the VST3 SDK repo and use GIT_TAG with the full hash for reproducibility.
  - Files: `CMakeLists.txt`
  - Verify: cmake -S . -B build-check to confirm configure succeeds

- [x] **T03: Skip cmake-build-config, structure-weak-boundary, and informational rules** `est:10min`
  Add justified skips to nfr-review.yaml for: (1) cmake-build-config (8 amber) — subdirectory CMakeLists inherit from root; (2) structure-weak-boundary (6 amber) — engine/plugin boundary is intentional and tested by engine-isolation CI job; (3) sample-readme-exists (1 amber) — not applicable to VST3 plugin project; (4) otel-test-observability (1 amber) — not applicable to C++ GTest suite. Each skip gets a comment explaining the rationale.
  - Files: `nfr-review.yaml`
  - Verify: yamllint nfr-review.yaml or manual inspection of skip list

- [x] **T04: Address adr-gap amber findings with GSD decisions** `est:15min`
  14 adr-gap amber findings flag undocumented architectural decisions. Either: (1) record the key decisions via gsd_decision_save (C++20, engine isolation, VST3 MIDI-only, CMake build system) which satisfies the NFR scanner, or (2) skip adr-gap with rationale that decisions are documented in IMPLEMENTATION_PLAN.md and CLAUDE.md. Choose the lighter approach that satisfies the scanner.
  - Files: `nfr-review.yaml`, `.gsd/DECISIONS.md`
  - Verify: NFR scanner shows zero adr-gap findings or rule is skipped

- [x] **T05: Run NFR review locally and verify zero red findings** `est:10min`
  Run the NFR review tool locally against the codebase to verify all red findings are resolved and all amber findings are either fixed or skipped. This catches issues before they hit the PR gate.
  - Verify: local NFR review run shows zero red and zero amber findings

## Files Likely Touched

- nfr-review.yaml
- CMakeLists.txt
- .gsd/DECISIONS.md
