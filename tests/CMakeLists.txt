include(FetchContent)
FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(poly_tests
    constraint_tests.cpp
    emission_tests.cpp
    euclidean_tests.cpp
    envelope_tests.cpp
    golden_tests.cpp
    dynamic_shaping_tests.cpp
    swing_humanize_tests.cpp
    macro_tests.cpp
    midi_capture_tests.cpp
    plugin_tests.cpp
    preset_tests.cpp
    sanitize_tests.cpp
    scene_tests.cpp
    smf_writer_tests.cpp
)

target_link_libraries(poly_tests
    PRIVATE poly_engine GTest::gtest_main
)

jk_target_warnings(poly_tests)

include(GoogleTest)
gtest_discover_tests(poly_tests)

# --- Fuzz Tests (require LLVM clang with libFuzzer) ---
if(BUILD_FUZZ_TESTS)
    add_executable(fuzz_state_io fuzz/fuzz_state_io.cpp)
    target_link_libraries(fuzz_state_io PRIVATE poly_engine)
    target_compile_features(fuzz_state_io PRIVATE cxx_std_20)
    target_compile_options(fuzz_state_io PRIVATE -fsanitize=fuzzer)
    target_link_options(fuzz_state_io PRIVATE -fsanitize=fuzzer)
    jk_target_warnings(fuzz_state_io)
endif()

# --- Bridge Contract Tests (require VST3 SDK for ParamID types) ---
if(NOT POLY_ENGINE_ONLY)
    FetchContent_Declare(nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG        v3.11.3
    )
    set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(nlohmann_json)

    FetchContent_Declare(choc
        GIT_REPOSITORY https://github.com/Tracktion/choc.git
        GIT_TAG 5685fb59db9e60d31a35df7bff0ce7967beaaf25
    )
    FetchContent_MakeAvailable(choc)

    add_executable(poly_bridge_tests
        bridge_contract_tests.cpp
        ${CMAKE_SOURCE_DIR}/plugin/source/webui/bridge_serialization.cpp
    )

    target_link_libraries(poly_bridge_tests PRIVATE
        poly_engine
        GTest::gtest_main
        sdk
        nlohmann_json::nlohmann_json
    )

    if(APPLE)
        target_link_libraries(poly_bridge_tests PRIVATE "-framework CoreFoundation")
    endif()

    target_include_directories(poly_bridge_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/plugin/source
        ${CMAKE_SOURCE_DIR}/engine/include
        ${choc_SOURCE_DIR}
    )

    target_compile_definitions(poly_bridge_tests PRIVATE
        FIXTURE_DIR="${CMAKE_SOURCE_DIR}/webui/tests/fixtures"
        SCHEMA_DIR="${CMAKE_SOURCE_DIR}/webui"
    )

    target_compile_features(poly_bridge_tests PRIVATE cxx_std_20)
    jk_target_warnings(poly_bridge_tests)
    gtest_discover_tests(poly_bridge_tests)
endif()

# --- Host Integration Tests (require VST3 SDK) ---
if(NOT POLY_ENGINE_ONLY)
    add_executable(poly_host_tests
        host/host_tests.cpp
        host/poly_test_host.cpp
        ${CMAKE_SOURCE_DIR}/plugin/source/processor.cpp
        ${vst3sdk_SOURCE_DIR}/public.sdk/source/common/memorystream.cpp
    )

    target_link_libraries(poly_host_tests PRIVATE
        poly_engine
        GTest::gtest_main
        sdk
        sdk_hosting
    )

    if(APPLE)
        target_link_libraries(poly_host_tests PRIVATE "-framework CoreFoundation")
    endif()

    target_include_directories(poly_host_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/plugin/source
        ${CMAKE_SOURCE_DIR}/engine/include
    )

    target_compile_definitions(poly_host_tests PRIVATE
        GOLDEN_DIR="${CMAKE_SOURCE_DIR}/tests/golden"
    )

    target_compile_features(poly_host_tests PRIVATE cxx_std_20)
    jk_target_warnings(poly_host_tests)
    gtest_discover_tests(poly_host_tests)

    # --- Probe Tests (require VST3 SDK + Poly processor + probe) ---
    add_executable(poly_probe_tests
        host/probe_tests.cpp
        host/poly_test_host.cpp
        ${CMAKE_SOURCE_DIR}/plugin/source/processor.cpp
        ${CMAKE_SOURCE_DIR}/tools/midi_probe/source/probe_processor.cpp
    )

    target_link_libraries(poly_probe_tests PRIVATE
        poly_engine
        GTest::gtest_main
        sdk
        sdk_hosting
    )

    if(APPLE)
        target_link_libraries(poly_probe_tests PRIVATE "-framework CoreFoundation")
    endif()

    target_include_directories(poly_probe_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/plugin/source
        ${CMAKE_SOURCE_DIR}/engine/include
        ${CMAKE_SOURCE_DIR}/tools/midi_probe/source
    )

    target_compile_features(poly_probe_tests PRIVATE cxx_std_20)
    jk_target_warnings(poly_probe_tests)
    gtest_discover_tests(poly_probe_tests)
endif()

# --- UI Tests (require VST3 SDK + VSTGUI) ---
if(NOT POLY_ENGINE_ONLY)
    include(${CMAKE_SOURCE_DIR}/cmake/jk_ui_test_harness.cmake)

    set(POLY_PLUGIN_SOURCES
        ${CMAKE_SOURCE_DIR}/plugin/source/controller.h
        ${CMAKE_SOURCE_DIR}/plugin/source/controller.cpp
        ${CMAKE_SOURCE_DIR}/plugin/source/plugids.h
        ${CMAKE_SOURCE_DIR}/plugin/source/ui/cell_editor_view.h
        ${CMAKE_SOURCE_DIR}/plugin/source/ui/cell_editor_view.cpp
        ${CMAKE_SOURCE_DIR}/plugin/source/ui/cross_rhythm_view.h
        ${CMAKE_SOURCE_DIR}/plugin/source/ui/cross_rhythm_view.cpp
        ${CMAKE_SOURCE_DIR}/plugin/source/ui/envelope_curve_view.h
        ${CMAKE_SOURCE_DIR}/plugin/source/ui/envelope_curve_view.cpp
        ${CMAKE_SOURCE_DIR}/plugin/source/ui/export_controls_view.h
        ${CMAKE_SOURCE_DIR}/plugin/source/ui/export_controls_view.cpp
        ${CMAKE_SOURCE_DIR}/plugin/source/ui/header_view.h
        ${CMAKE_SOURCE_DIR}/plugin/source/ui/header_view.cpp
        ${CMAKE_SOURCE_DIR}/plugin/source/ui/lane_edit_view.h
        ${CMAKE_SOURCE_DIR}/plugin/source/ui/lane_edit_view.cpp
        ${CMAKE_SOURCE_DIR}/plugin/source/ui/lane_grid_view.h
        ${CMAKE_SOURCE_DIR}/plugin/source/ui/lane_grid_view.cpp
        ${CMAKE_SOURCE_DIR}/plugin/source/ui/phase_alignment_view.h
        ${CMAKE_SOURCE_DIR}/plugin/source/ui/phase_alignment_view.cpp
        ${CMAKE_SOURCE_DIR}/plugin/source/ui/micro_timing_editor_view.h
        ${CMAKE_SOURCE_DIR}/plugin/source/ui/micro_timing_editor_view.cpp
        ${CMAKE_SOURCE_DIR}/plugin/source/ui/note_map_view.h
        ${CMAKE_SOURCE_DIR}/plugin/source/ui/note_map_view.cpp
        ${CMAKE_SOURCE_DIR}/plugin/source/ui/timeline_step_editor_view.h
        ${CMAKE_SOURCE_DIR}/plugin/source/ui/timeline_step_editor_view.cpp
        ${CMAKE_SOURCE_DIR}/plugin/source/ui/phrase_edit_view.h
        ${CMAKE_SOURCE_DIR}/plugin/source/ui/phrase_edit_view.cpp
        ${CMAKE_SOURCE_DIR}/plugin/source/ui/scene_bar_view.h
        ${CMAKE_SOURCE_DIR}/plugin/source/ui/scene_bar_view.cpp
        ${CMAKE_SOURCE_DIR}/plugin/source/ui/chain_popover_view.h
        ${CMAKE_SOURCE_DIR}/plugin/source/ui/chain_popover_view.cpp
        ${CMAKE_SOURCE_DIR}/plugin/source/ui/velocity_view.h
        ${CMAKE_SOURCE_DIR}/plugin/source/ui/velocity_view.cpp
    )

    if(BUILD_VISUAL_TESTS)
        set(VISUAL_TEST_HARNESS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ui/visual")
        set(VISUAL_TEST_REFERENCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ui/visual/references")
        set(VISUAL_TEST_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/visual_output")

        add_executable(poly_visual_tests
            ui/visual/visual_smoke_tests.cpp
            ${POLY_PLUGIN_SOURCES}
        )

        jk_add_visual_tests(poly_visual_tests)

        target_link_libraries(poly_visual_tests PRIVATE
            poly_engine
            GTest::gtest_main
            sdk
            vstgui_support
        )

        target_include_directories(poly_visual_tests PRIVATE
            ${CMAKE_SOURCE_DIR}/plugin/source
            ${CMAKE_SOURCE_DIR}/engine/include
        )

        target_compile_features(poly_visual_tests PRIVATE cxx_std_20)
        jk_target_warnings(poly_visual_tests)
        gtest_discover_tests(poly_visual_tests)
    endif()

    if(BUILD_INTERACTION_TESTS AND NOT WIN32)
        set(INTERACTION_TEST_HARNESS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ui/interaction")

        add_executable(poly_interaction_tests
            ui/interaction/interaction_smoke_tests.cpp
            ${POLY_PLUGIN_SOURCES}
        )

        jk_add_interaction_tests(poly_interaction_tests)

        target_link_libraries(poly_interaction_tests PRIVATE
            poly_engine
            GTest::gtest_main
            sdk
            sdk_hosting
            vstgui_support
        )

        target_include_directories(poly_interaction_tests PRIVATE
            ${CMAKE_SOURCE_DIR}/plugin/source
            ${CMAKE_SOURCE_DIR}/engine/include
        )

        target_compile_definitions(poly_interaction_tests PRIVATE
            UIDESC_RESOURCE_DIR="${CMAKE_SOURCE_DIR}/plugin/resource"
        )

        # On Linux/Windows, vstgui_support requires VST3 module init symbols
        # (getPlatformModuleHandle, ModuleInitializer, ModuleTerminator).
        # macOS uses -Wl,-undefined,dynamic_lookup instead.
        if(NOT APPLE)
            target_sources(poly_interaction_tests PRIVATE
                ${vst3sdk_SOURCE_DIR}/public.sdk/source/main/moduleinit.cpp
            )
        endif()

        target_compile_features(poly_interaction_tests PRIVATE cxx_std_20)
        jk_target_warnings(poly_interaction_tests)
        gtest_discover_tests(poly_interaction_tests)
    endif()
endif()
