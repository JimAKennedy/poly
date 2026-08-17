cmake_minimum_required(VERSION 3.14.0)

# The vendored VST3 SDK (fetched below) still ships sub-project CMakeLists with
# `cmake_minimum_required(VERSION 3.x)` where x < 3.10 (e.g. base/CMakeLists.txt),
# which CMake >= 3.31 flags with a deprecation warning and CMake 4.x will hard-
# error on. We do not control those third-party files (FetchContent re-fetches
# them), so raise the *minimum* policy version any stale `cmake_minimum_required`
# is allowed to request. 3.10 is the lowest value that clears the deprecation
# without silently upgrading policies we haven't audited. This propagates into
# the FetchContent sub-builds, so no per-configure -D flag is needed in the build
# script or CI workflows. Our own policies stay governed by the 3.14 line above.
if(NOT DEFINED CMAKE_POLICY_VERSION_MINIMUM)
    set(CMAKE_POLICY_VERSION_MINIMUM 3.10 CACHE STRING
        "Minimum policy version for stale third-party cmake_minimum_required calls")
endif()

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

# Phase 1 of the jk-standards warning-flags adoption: consume the standard's
# diagnostic roster now, but do not yet promote it to fatal. Adopting the full
# set surfaces ~242 pre-existing signedness/float-conversion warnings across the
# engine and tests; making them fatal today would leave the build red. This
# option keeps them visible-but-non-fatal until the burndown lands, at which
# point flipping the default to ON completes conformance. Tracked in the
# adoption PR / follow-up issue.
option(POLY_WARNINGS_FATAL "Treat the jk-standards warning set as errors (-Werror/-WX)" OFF)

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
# cmake/jk_warnings.cmake is VENDORED verbatim from jk-standards
# (conventions/warning-flags.md), the reference implementation of the gated
# warning-flags standard, adopted via the copy-with-checksum path. The checksum
# keeps the vendored copy honest: a local edit silently forks poly's warning set
# from the standard, and the fork stays invisible until a diagnostic that should
# have failed a build quietly does not. To update, re-vendor from the upstream
# tag and refresh JK_WARNINGS_SHA256.
#   Upstream: https://github.com/JimAKennedy/jk-standards cmake/jk_warnings.cmake @ v0.5.0
set(JK_WARNINGS_SHA256 "cde085e90f2efe9feb84cbb9f79d43a29e21b8d3169efc83fb996f336008701b")
file(SHA256 "${CMAKE_CURRENT_SOURCE_DIR}/cmake/jk_warnings.cmake" _jk_warnings_actual_sha)
if(NOT _jk_warnings_actual_sha STREQUAL JK_WARNINGS_SHA256)
    message(FATAL_ERROR
        "cmake/jk_warnings.cmake has drifted from the pinned jk-standards module "
        "(expected ${JK_WARNINGS_SHA256}, got ${_jk_warnings_actual_sha}). "
        "Re-vendor from the upstream tag and update JK_WARNINGS_SHA256.")
endif()
include(cmake/jk_warnings.cmake)

# Phase-1 wrapper: apply the standard's roster, then demote -Werror/-WX to
# non-fatal until POLY_WARNINGS_FATAL is turned on. CMake exposes the just-
# defined jk_target_warnings under the _jk_target_warnings alias when we
# override the name, so we delegate to the vendored implementation and only add
# the demotion flags. Flip POLY_WARNINGS_FATAL ON (or make it the default) once
# the warning burndown is complete to reach full conformance with the standard.
function(jk_target_warnings target)
    _jk_target_warnings(${target})
    if(NOT POLY_WARNINGS_FATAL AND TARGET ${target})
        get_target_property(_jkw_type ${target} TYPE)
        if(NOT _jkw_type STREQUAL "INTERFACE_LIBRARY")
            target_compile_options(${target} PRIVATE
                $<$<CXX_COMPILER_ID:MSVC>:/WX->
                $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:-Wno-error>)
        endif()
    endif()
endfunction()

if(NOT POLY_ENGINE_ONLY)
    # --- VST3 SDK via FetchContent ---
    include(FetchContent)
    FetchContent_Declare(vst3sdk
        GIT_REPOSITORY https://github.com/steinbergmedia/vst3sdk.git
        GIT_TAG        cc2adc90382dded9e347caf74e4532f1458715db # v3.7.12_build_20
    )

    set(SMTG_ENABLE_VST3_HOSTING_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(SMTG_ENABLE_VST3_PLUGIN_EXAMPLES  OFF CACHE BOOL "" FORCE)
    # No first-party target links VSTGUI after M053/S05 (native editor deleted;
    # WebUI is choc-webview). Turn SDK VSTGUI support off so the vstgui/
    # vstgui_uidescription/vstgui_support targets are never configured or built.
    set(SMTG_ENABLE_VSTGUI_SUPPORT        OFF CACHE BOOL "" FORCE)
    set(SMTG_RUN_VST_VALIDATOR            OFF CACHE BOOL "Run VST3 validator as post-build step")
    set(SMTG_CREATE_PLUGIN_LINK           OFF CACHE BOOL "" FORCE)

    # --- AudioUnit v2 support (macOS + Xcode generator ONLY) ---
    # The Steinberg AU wrapper (public.sdk/source/vst/auwrapper) is hard-gated on
    # `XCODE AND SMTG_ENABLE_AUV2_BUILDS` in SMTG_AddVST3AuV2.cmake, and the SDK's
    # setupCoreAudioSupport() only enables AUv2 when it can locate an AudioUnitSDK.
    # We fetch Apple's AudioUnitSDK and point SMTG_AUDIOUNIT_SDK_PATH at it BEFORE
    # the vst3sdk is made available, so its setupCoreAudioSupport() picks it up and
    # flips SMTG_ENABLE_AUV2_BUILDS on. This whole block is skipped under the Ninja
    # generator (XCODE is false there), so the existing VST3/test/pluginval CI legs
    # are completely unaffected — the AU path is purely additive.
    if(XCODE)
        FetchContent_Declare(audiounitsdk
            GIT_REPOSITORY https://github.com/apple/AudioUnitSDK.git
            # Apple tags releases as `AudioUnitSDK-<ver>`, not a bare `<ver>` —
            # the bare `1.1.0` is not a valid ref and the clone fails once the
            # FetchContent cache misses (see M051 CI: cache-miss surfaced it).
            GIT_TAG        AudioUnitSDK-1.1.0
            # AudioUnitSDK is consumed as a source tree (its AudioUnitSDK.xcodeproj is
            # built by the wrapper's PRE_BUILD step). Steer FetchContent away from
            # add_subdirectory so it never injects its own targets into our build.
            SOURCE_SUBDIR  do-not-add-this-subdir
        )
        FetchContent_MakeAvailable(audiounitsdk)
        set(SMTG_AUDIOUNIT_SDK_PATH "${audiounitsdk_SOURCE_DIR}" CACHE PATH "" FORCE)
        set(SMTG_ENABLE_AUV2_BUILDS ON CACHE BOOL "" FORCE)
        # The AU CI leg configures with -DCMAKE_OSX_ARCHITECTURES=arm64 (single
        # arch), but the SDK defaults SMTG_BUILD_UNIVERSAL_BINARY=ON, which forces
        # x86_64;arm64 onto the SDK/plugin/AU targets via smtg_target_setup_universal_binary.
        # poly_engine honours the arm64-only global, so the plugin's x86_64 slice
        # fails to find poly_engine symbols at link. Force single-arch to match.
        set(SMTG_BUILD_UNIVERSAL_BINARY OFF CACHE BOOL "" FORCE)
        # Ship the component ad-hoc signed (per D004 — no Developer-ID/notarization
        # yet, and GitHub runners have no "Apple Development" identity). The SDK's
        # default codesign identity would fail the poly_au post-build codesign; "-"
        # selects ad-hoc, matching the -adhoc_codesign already applied at link.
        set(SMTG_CODE_SIGN_IDENTITY_MAC "-" CACHE STRING "" FORCE)
    endif()

    FetchContent_MakeAvailable(vst3sdk)

    # Isolate the third-party VST3 SDK targets from the first-party warning set.
    # The jk-standards module's jk_suppress_sdk_warnings() takes one target (it
    # FATAL_ERRORs on a non-target), so iterate the SDK targets we build and
    # suppress only the ones this configuration actually defined.
    foreach(_sdk_target
        vstgui vstgui_uidescription vstgui_support
        sdk sdk_common sdk_hosting base pluginterfaces)
        if(TARGET ${_sdk_target})
            jk_suppress_sdk_warnings(${_sdk_target})
        endif()
    endforeach()
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
        "_poly_emission_count"
        "_poly_emission_buffer"
        "_poly_emission_fields_per_event"
        "_poly_active_lane_count"
        "_poly_macro_value"
        "_poly_set_macro"
        "_poly_edit_lane_int"
        "_poly_edit_lane_float"
        "_poly_lane_int"
        "_poly_lane_float"
        "_poly_lane_pattern"
        "_poly_lane_micro_timing_ptr"
        "_poly_lane_accents_ptr"
        "_poly_lane_cells_ptr"
        "_poly_lane_fixed_pattern"
        "_poly_lane_envelope_count"
        "_poly_lane_envelope_active"
        "_poly_lane_envelope_target"
        "_poly_lane_envelope_period"
        "_poly_lane_envelope_depth"
        "_poly_import_midi"
        "_poly_revert_import"
        "_poly_action_set_euclid"
        "_poly_action_toggle_step"
        "_poly_action_apply_preset"
        "_poly_action_apply_preset_to_scene"
        "_poly_action_select_scene"
        "_poly_scene_select"
        "_poly_set_morph"
        "_poly_morph_amount"
        "_poly_copy_scenes"
        "_poly_action_set_fixed_step"
        "_poly_action_set_micro_timing"
        "_poly_action_set_accent"
        "_poly_action_set_cells"
        "_poly_action_clear_cells"
        "_poly_action_set_envelope"
        "_poly_action_remove_envelope"
        "_poly_seed"
        "_poly_set_seed"
        "_poly_set_chain_enabled"
        "_poly_chain_enabled"
        "_poly_set_chain_mode"
        "_poly_chain_mode"
        "_poly_chain_entry_count"
        "_poly_chain_add_entry"
        "_poly_chain_remove_entry"
        "_poly_set_chain_entry_scene"
        "_poly_set_chain_entry_bars"
        "_poly_chain_entry_scene"
        "_poly_chain_entry_bars"
        "_malloc"
        "_free"
    )
    list(JOIN POLY_EXPORTED_FUNCTIONS "," POLY_EXPORTED_FUNCTIONS_STR)

    target_link_options(poly_wasm PRIVATE
        "SHELL:-s MODULARIZE=1"
        "SHELL:-s EXPORT_ES6=1"
        "SHELL:-s EXPORTED_FUNCTIONS=[${POLY_EXPORTED_FUNCTIONS_STR}]"
        "SHELL:-s EXPORTED_RUNTIME_METHODS=[ccall,cwrap,getValue,UTF8ToString,HEAP8,HEAPU8,HEAP16,HEAPU16,HEAP32,HEAPU32,HEAPF32,HEAPF64]"
        "SHELL:-s ALLOW_MEMORY_GROWTH=0"
        "SHELL:-s INITIAL_MEMORY=16777216"
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
