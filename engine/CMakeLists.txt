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
    src/sanitize.cpp
    src/scene.cpp
    src/smf_writer.cpp
    src/wasm_api.cpp
)

target_include_directories(poly_engine
    PUBLIC include
)

jk_target_warnings(poly_engine)

if(NOT EMSCRIPTEN)
    add_subdirectory(tools)
endif()
