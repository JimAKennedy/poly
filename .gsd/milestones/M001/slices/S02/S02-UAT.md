# S02: Engine Domain Model and Headless Harness — UAT

**Milestone:** M001
**Written:** 2026-06-20T19:57:15.832Z

## UAT: M001 S02 — Engine Domain Model and Headless Harness\n\n- [ ] `cmake --build build --target poly_engine` compiles with zero warnings\n- [ ] `cmake --build build --target poly_harness` compiles and links\n- [ ] `./build/tools/harness/poly_harness` runs and prints header + event lines\n- [ ] `nm build/engine/libpoly_engine.a | grep -i steinberg` returns empty\n- [ ] Domain model types match IMPLEMENTATION_PLAN.md §3 spec
