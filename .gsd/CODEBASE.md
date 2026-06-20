# Codebase Map

Generated: 2026-06-20T21:35:30Z | Files: 51 | Described: 0/51
<!-- gsd:codebase-meta {"generatedAt":"2026-06-20T21:35:30Z","fingerprint":"d39f04a0e45c7dc2e9bee33dd994bf4e75d2a9a3","fileCount":51,"truncated":false} -->

### (root)/
- `.clang-format`
- `.clang-tidy`
- `.cmakelintrc`
- `.gitignore`
- `.gitleaks.toml`
- `.pre-commit-config.yaml`
- `CLAUDE.md`
- `CMakeLists.txt`
- `IMPLEMENTATION_PLAN.md`

### .github/
- `.github/lsan.supp`
- `.github/tsan.supp`
- `.github/ubsan.supp`

### .github/workflows/
- `.github/workflows/ci.yml`
- `.github/workflows/sanitizers.yml`

### cmake/
- `cmake/jk_warnings.cmake`

### docs/
- `docs/automation-mapping.md`
- `docs/cubase-workflow.md`
- `docs/engine-spec.md`
- `docs/PRD.md`
- `docs/roadmap.md`
- `docs/wireframes.md`

### docs/review/
- `docs/review/api-audit.md`
- `docs/review/architecture-decisions.md`
- `docs/review/retrospective.md`
- `docs/review/test-coverage.md`

### engine/
- `engine/CMakeLists.txt`

### engine/include/poly/
- `engine/include/poly/engine.h`
- `engine/include/poly/envelope.h`
- `engine/include/poly/euclidean.h`
- `engine/include/poly/rng.h`
- `engine/include/poly/types.h`

### engine/src/
- `engine/src/engine.cpp`
- `engine/src/envelope.cpp`
- `engine/src/euclidean.cpp`

### plugin/
- `plugin/CMakeLists.txt`

### plugin/source/
- `plugin/source/controller.cpp`
- `plugin/source/controller.h`
- `plugin/source/factory.cpp`
- `plugin/source/plugids.h`
- `plugin/source/processor.cpp`
- `plugin/source/processor.h`

### scripts/
- `scripts/check-pragma-once.sh`
- `scripts/check-realtime-safety.sh`

### tests/
- `tests/CMakeLists.txt`
- `tests/dynamic_shaping_tests.cpp`
- `tests/envelope_tests.cpp`
- `tests/euclidean_tests.cpp`
- `tests/golden_tests.cpp`

### tests/golden/
- `tests/golden/default_patch_4bars.txt`

### tools/harness/
- `tools/harness/CMakeLists.txt`
- `tools/harness/main.cpp`
