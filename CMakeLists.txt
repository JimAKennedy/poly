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
endif()

enable_testing()
add_subdirectory(tests)
