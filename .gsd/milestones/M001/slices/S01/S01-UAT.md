# S01: Build System and VST3 Skeleton — UAT

**Milestone:** M001
**Written:** 2026-06-20T19:52:24.862Z

## UAT: Build System and VST3 Skeleton\n\n### Checks\n- [ ] `cmake -B build -DCMAKE_BUILD_TYPE=Debug` completes without error\n- [ ] `cmake --build build` produces `build/VST3/Debug/poly_plugin.vst3`\n- [ ] No warnings from `engine/` or `plugin/source/` files\n- [ ] `poly_engine` compiles independently (no VST3 SDK includes)\n- [ ] .vst3 bundle loads in Cubase without crash (manual test)
