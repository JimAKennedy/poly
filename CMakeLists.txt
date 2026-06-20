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

# --- VST3 SDK via FetchContent ---
include(FetchContent)
FetchContent_Declare(vst3sdk
    GIT_REPOSITORY https://github.com/steinbergmedia/vst3sdk.git
    GIT_TAG        v3.7.12_build_20
)

set(SMTG_ENABLE_VST3_HOSTING_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SMTG_ENABLE_VST3_PLUGIN_EXAMPLES  OFF CACHE BOOL "" FORCE)
set(SMTG_ENABLE_VSTGUI_SUPPORT        ON  CACHE BOOL "" FORCE)
set(SMTG_RUN_VST_VALIDATOR            OFF CACHE BOOL "" FORCE)
set(SMTG_CREATE_PLUGIN_LINK           OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(vst3sdk)

# --- Warnings ---
include(cmake/jk_warnings.cmake)
jk_suppress_sdk_warnings()

# --- Subdirectories ---
add_subdirectory(engine)
add_subdirectory(plugin)
