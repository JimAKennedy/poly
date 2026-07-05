cmake_minimum_required(VERSION 3.14.0)

set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "")

project(poly
    VERSION 0.1.0
    DESCRIPTION "Polymetric drum pattern generator"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Build options ---
option(POLY_ENGINE_ONLY "Build only the engine library and tests (no VST3 SDK)" OFF)

if(EMSCRIPTEN)
    set(POLY_ENGINE_ONLY ON CACHE BOOL "" FORCE)
endif()
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)
option(ENABLE_COVERAGE "Enable code coverage" OFF)
option(BUILD_VISUAL_TESTS "Build visual regression tests" OFF)
option(BUILD_INTERACTION_TESTS "Build headless UI interaction tests" OFF)

if(ENABLE_ASAN)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
elseif(ENABLE_UBSAN)
    add_compile_options(-fsanitize=undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=undefined)
elseif(ENABLE_TSAN)
    add_compile_options(-fsanitize=thread)
    add_link_options(-fsanitize=thread)
endif()

if(ENABLE_COVERAGE)
    add_compile_options(--coverage -fprofile-arcs -ftest-coverage)
    add_link_options(--coverage)
endif()

# --- Warnings ---
include(cmake/jk_warnings.cmake)

if(NOT POLY_ENGINE_ONLY)
    # --- VST3 SDK via FetchContent ---
    include(FetchContent)
    FetchContent_Declare(vst3sdk
        GIT_REPOSITORY https://github.com/steinbergmedia/vst3sdk.git
        GIT_TAG        cc2adc90382dded9e347caf74e4532f1458715db # v3.7.12_build_20
    )

    set(SMTG_ENABLE_VST3_HOSTING_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(SMTG_ENABLE_VST3_PLUGIN_EXAMPLES  OFF CACHE BOOL "" FORCE)
    set(SMTG_ENABLE_VSTGUI_SUPPORT        ON  CACHE BOOL "" FORCE)
    set(SMTG_RUN_VST_VALIDATOR            OFF CACHE BOOL "Run VST3 validator as post-build step")
    set(SMTG_CREATE_PLUGIN_LINK           OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(vst3sdk)
    jk_suppress_sdk_warnings()
endif()

# --- Subdirectories ---
add_subdirectory(engine)

if(NOT POLY_ENGINE_ONLY)
    add_subdirectory(plugin)
    add_subdirectory(tools/harness)
    add_subdirectory(tools/midi_probe)
endif()

# --- WASM build (Emscripten) ---
if(EMSCRIPTEN)
    add_executable(poly_wasm engine/src/wasm_api.cpp)
    target_link_libraries(poly_wasm PRIVATE poly_engine)
    target_include_directories(poly_wasm PRIVATE engine/include)

    set(POLY_EXPORTED_FUNCTIONS
        "_poly_create"
        "_poly_destroy"
        "_poly_preset_count"
        "_poly_preset_name"
        "_poly_preset_description"
        "_poly_load_preset"
        "_poly_render"
        "_poly_event_count"
        "_poly_event_buffer"
        "_poly_active_lane_count"
        "_poly_macro_value"
        "_poly_set_macro"
        "_poly_edit_lane_int"
        "_poly_edit_lane_float"
        "_poly_action_set_euclid"
        "_poly_action_toggle_step"
        "_poly_action_apply_preset"
        "_poly_seed"
        "_poly_set_seed"
        "_malloc"
        "_free"
    )
    list(JOIN POLY_EXPORTED_FUNCTIONS "," POLY_EXPORTED_FUNCTIONS_STR)

    target_link_options(poly_wasm PRIVATE
        "SHELL:-s MODULARIZE=1"
        "SHELL:-s EXPORT_ES6=1"
        "SHELL:-s EXPORTED_FUNCTIONS=[${POLY_EXPORTED_FUNCTIONS_STR}]"
        "SHELL:-s EXPORTED_RUNTIME_METHODS=[ccall,cwrap,getValue,UTF8ToString]"
        "SHELL:-s ALLOW_MEMORY_GROWTH=1"
        "SHELL:-s INITIAL_MEMORY=4194304"
        "SHELL:-s NO_EXIT_RUNTIME=1"
        "SHELL:-s ENVIRONMENT=web"
        -O2
    )

    set_target_properties(poly_wasm PROPERTIES
        OUTPUT_NAME "poly_engine"
        SUFFIX ".js"
    )
endif()

enable_testing()
add_subdirectory(tests)
