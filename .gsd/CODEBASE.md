# Codebase Map

Generated: 2026-06-20T21:28:35Z | Files: 48 | Described: 0/48
<!-- gsd:codebase-meta {"generatedAt":"2026-06-20T21:28:35Z","fingerprint":"58b36bb851d093b37f0d27c7ee5f1f1272ab8bbe","fileCount":48,"truncated":false} -->

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
- `engine/include/poly/euclidean.h`
- `engine/include/poly/rng.h`
- `engine/include/poly/types.h`

### engine/src/
- `engine/src/engine.cpp`
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
- `tests/euclidean_tests.cpp`
- `tests/golden_tests.cpp`

### tests/golden/
- `tests/golden/default_patch_4bars.txt`

### tools/harness/
- `tools/harness/CMakeLists.txt`
- `tools/harness/main.cpp`
