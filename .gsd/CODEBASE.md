# Codebase Map

Generated: 2026-06-21T13:04:37Z | Files: 80 | Described: 0/80
<!-- gsd:codebase-meta {"generatedAt":"2026-06-21T13:04:37Z","fingerprint":"847b75319ca65f755a1bf7de1b532d0a3e652465","fileCount":80,"truncated":false} -->

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
- `cmake/jk_ui_test_harness.cmake`
- `cmake/jk_warnings.cmake`

### docs/
- `docs/automation-mapping.md`
- `docs/cubase-workflow.md`
- `docs/engine-spec.md`
- `docs/M002-UAT-SCRIPT.md`
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
- `engine/include/poly/bridge.h`
- `engine/include/poly/constraint.h`
- `engine/include/poly/engine.h`
- `engine/include/poly/envelope.h`
- `engine/include/poly/euclidean.h`
- `engine/include/poly/macro.h`
- `engine/include/poly/rng.h`
- `engine/include/poly/scene.h`
- `engine/include/poly/state_io.h`
- `engine/include/poly/types.h`

### engine/src/
- `engine/src/bridge.cpp`
- `engine/src/constraint.cpp`
- `engine/src/engine.cpp`
- `engine/src/envelope.cpp`
- `engine/src/euclidean.cpp`
- `engine/src/macro.cpp`
- `engine/src/scene.cpp`

### plugin/
- `plugin/CMakeLists.txt`

### plugin/resource/
- `plugin/resource/poly.uidesc`

### plugin/source/
- `plugin/source/controller.cpp`
- `plugin/source/controller.h`
- `plugin/source/factory.cpp`
- `plugin/source/plugids.h`
- `plugin/source/processor.cpp`
- `plugin/source/processor.h`

### plugin/source/ui/
- `plugin/source/ui/lane_grid_view.cpp`
- `plugin/source/ui/lane_grid_view.h`
- `plugin/source/ui/velocity_view.cpp`
- `plugin/source/ui/velocity_view.h`

### scripts/
- `scripts/check-pragma-once.sh`
- `scripts/check-realtime-safety.sh`

### tests/
- `tests/CMakeLists.txt`
- `tests/constraint_tests.cpp`
- `tests/dynamic_shaping_tests.cpp`
- `tests/envelope_tests.cpp`
- `tests/euclidean_tests.cpp`
- `tests/golden_tests.cpp`
- `tests/macro_tests.cpp`
- `tests/plugin_tests.cpp`
- `tests/scene_tests.cpp`
- `tests/swing_humanize_tests.cpp`

### tests/golden/
- `tests/golden/default_patch_4bars.txt`

### tests/ui/interaction/
- `tests/ui/interaction/headless_ui_host.cpp`
- `tests/ui/interaction/headless_ui_host.h`
- `tests/ui/interaction/interaction_smoke_tests.cpp`

### tests/ui/visual/
- `tests/ui/visual/image_compare.cpp`
- `tests/ui/visual/image_compare.h`
- `tests/ui/visual/visual_smoke_tests.cpp`
- `tests/ui/visual/visual_test_harness.cpp`
- `tests/ui/visual/visual_test_harness.h`

### tools/harness/
- `tools/harness/CMakeLists.txt`
- `tools/harness/main.cpp`
