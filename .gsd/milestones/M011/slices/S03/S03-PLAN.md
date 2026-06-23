# S03: Resolve remaining NFR red and amber findings

**Goal:** Address ci-security-scan-missing red finding and cmake amber findings to reach zero red and zero amber in NFR review
**Demo:** NFR review shows zero red and zero amber findings

## Must-Haves

- Complete the planned slice outcomes.

## Verification

- Run the task and slice verification checks for this slice.

## Tasks

- [ ] **T01: Add CodeQL SAST scanning to CI or skip with rationale** `est:20min`
  NFR review flags ci-security-scan-missing (red/high). Options:
  1. Add a CodeQL workflow for C++ scanning (GitHub provides free CodeQL for public repos). Configure with proper exclusions for build/_deps/ and VST3 SDK code.
  2. Skip the rule in nfr-review.yaml with documented rationale (e.g. sanitizers workflow already covers memory safety; gitleaks covers secrets).
  - Files: `.github/workflows/ci.yml`, `nfr-review.yaml`
  - Verify: yamllint .github/workflows/ci.yml (syntax check); or gh workflow run ci.yml --dry-run if possible

- [ ] **T02: Pin VST3 SDK FetchContent to commit hash** `est:10min`
  cmake-fetchcontent-pinning (amber/medium) flags that VST3 SDK uses tag `v3.7.12_build_20` instead of a commit hash in CMakeLists.txt:48. Resolve the tag to its commit SHA and use GIT_TAG with the full hash for reproducibility.
  - Files: `CMakeLists.txt`
  - Verify: cmake -S . -B build-check (configure succeeds with commit hash)

- [ ] **T03: Skip or fix cmake-build-config amber findings** `est:10min`
  8 amber/low cmake-build-config findings for subdirectory CMakeLists missing project() VERSION and target_compile_features. These subdirectories (engine, plugin, tools/harness) inherit from the root CMakeLists.txt which sets the C++ standard via CMAKE_CXX_STANDARD 20.
  - Files: `nfr-review.yaml`
  - Verify: Read nfr-review.yaml to confirm rule is in skip list with comment

## Files Likely Touched

- .github/workflows/ci.yml
- nfr-review.yaml
- CMakeLists.txt
