# Codebase Map

Generated: 2026-06-25T01:25:57Z | Files: 152 | Described: 0/152
<!-- gsd:codebase-meta {"generatedAt":"2026-06-25T01:25:57Z","fingerprint":"df9a32e1cf244dac187a7c2a33f44eec48d19bd0","fileCount":152,"truncated":false} -->

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
- `nfr-review.yaml`

### .github/
- `.github/lsan.supp`
- `.github/tsan.supp`
- `.github/ubsan.supp`

### .github/workflows/
- `.github/workflows/ci.yml`
- `.github/workflows/deploy-site.yml`
- `.github/workflows/nfr-review-nightly.yml`
- `.github/workflows/nfr-review.yml`
- `.github/workflows/sanitizers.yml`

### cmake/
- `cmake/jk_ui_test_harness.cmake`
- `cmake/jk_warnings.cmake`

### docs/
- `docs/automation-mapping.md`
- `docs/cubase-workflow.md`
- `docs/engine-spec.md`
- `docs/euclidean-rhythm-guide.md`
- `docs/M002-UAT-SCRIPT.md`
- `docs/midi-note-mapping.md`
- `docs/PRD.md`
- `docs/roadmap.md`
- `docs/UAT-SCRIPT.md`
- `docs/ui-guide.md`
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
- `engine/include/poly/midi_capture.h`
- `engine/include/poly/presets.h`
- `engine/include/poly/rng.h`
- `engine/include/poly/scene.h`
- `engine/include/poly/smf_writer.h`
- `engine/include/poly/state_io.h`
- `engine/include/poly/types.h`

### engine/src/
- `engine/src/bridge.cpp`
- `engine/src/constraint.cpp`
- `engine/src/engine.cpp`
- `engine/src/envelope.cpp`
- `engine/src/euclidean.cpp`
- `engine/src/macro.cpp`
- `engine/src/midi_capture.cpp`
- `engine/src/scene.cpp`
- `engine/src/smf_writer.cpp`

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
- *(24 files: 12 .cpp, 12 .h)*

### scripts/
- `scripts/check-pragma-once.sh`
- `scripts/check-realtime-safety.sh`
- `scripts/pre-push-check.sh`

### site/
- `site/.gitignore`
- `site/AGENTS.md`
- `site/astro.config.mjs`
- `site/CLAUDE.md`
- `site/package-lock.json`
- `site/package.json`
- `site/README.md`
- `site/tsconfig.json`

### site/src/
- `site/src/content.config.ts`

### site/src/components/
- `site/src/components/CodeSnippet.astro`
- `site/src/components/EuclideanDiagram.astro`
- `site/src/components/ListenFor.astro`
- `site/src/components/PolyPatch.astro`
- `site/src/components/PolyScreenshot.astro`

### site/src/content/docs/
- `site/src/content/docs/01-foundations.mdx`
- `site/src/content/docs/02-sub-saharan-africa.mdx`
- `site/src/content/docs/03-afro-cuban.mdx`
- `site/src/content/docs/04-afrobeat.mdx`
- `site/src/content/docs/05-gamelan.mdx`
- `site/src/content/docs/06-indian-classical.mdx`
- `site/src/content/docs/07-balkan.mdx`
- `site/src/content/docs/08-minimalism.mdx`
- `site/src/content/docs/09-electronic.mdx`
- `site/src/content/docs/10-brazilian.mdx`
- `site/src/content/docs/11-funk-soul.mdx`
- `site/src/content/docs/12-jazz.mdx`
- `site/src/content/docs/13-drum-and-bass.mdx`
- `site/src/content/docs/14-synthesis.mdx`
- `site/src/content/docs/15-compositional-grammar.mdx`
- `site/src/content/docs/appendix-euclidean-reference.mdx`
- `site/src/content/docs/appendix-presets.mdx`
- `site/src/content/docs/component-demo.mdx`
- `site/src/content/docs/index.mdx`
- `site/src/content/docs/introduction.mdx`

### site/src/styles/
- `site/src/styles/custom.css`

### tests/
- `tests/CMakeLists.txt`
- `tests/constraint_tests.cpp`
- `tests/dynamic_shaping_tests.cpp`
- `tests/envelope_tests.cpp`
- `tests/euclidean_tests.cpp`
- `tests/golden_tests.cpp`
- `tests/macro_tests.cpp`
- `tests/midi_capture_tests.cpp`
- `tests/plugin_tests.cpp`
- `tests/preset_tests.cpp`
- `tests/scene_tests.cpp`
- `tests/smf_writer_tests.cpp`
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
