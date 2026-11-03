add_library(poly_engine STATIC
    src/bridge.cpp
    src/constraint.cpp
    src/engine.cpp
    src/envelope.cpp
    src/euclidean.cpp
    src/fitter.cpp
    src/lane_name.cpp
    src/macro.cpp
    src/midi_capture.cpp
    src/midi_reader.cpp
    src/offline_render.cpp
    src/presets.cpp
    src/presets_central_africa.cpp
    src/sanitize.cpp
    src/scene.cpp
    src/smf_writer.cpp
    src/wasm_api.cpp
)

target_include_directories(poly_engine
    PUBLIC include
)

jk_target_warnings(poly_engine)

# Determinism: forbid floating-point contraction (a*b+c fused into one FMA).
# GCC defaults to -ffp-contract=fast and contracts across statements whenever
# the target has FMA, so the same source rounds differently by compiler and
# CPU. ubuntu-26.04's GCC 15.2 did this to fitter.cpp's `onset - slot * g`,
# leaving a ~1e-32 onsetError where every other build computes exactly 0.
# PUBLIC so inline math in engine headers compiles the same way in the
# tests, the plugin and the WASM build. MSVC does not contract under its
# default /fp:precise.
target_compile_options(poly_engine
    PUBLIC $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-ffp-contract=off>
)

if(NOT EMSCRIPTEN)
    add_subdirectory(tools)
endif()
