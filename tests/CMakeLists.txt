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
    euclidean_tests.cpp
    envelope_tests.cpp
    golden_tests.cpp
    dynamic_shaping_tests.cpp
    swing_humanize_tests.cpp
    macro_tests.cpp
    plugin_tests.cpp
    scene_tests.cpp
)

target_link_libraries(poly_tests
    PRIVATE poly_engine GTest::gtest_main
)

jk_target_warnings(poly_tests)

include(GoogleTest)
gtest_discover_tests(poly_tests)

# --- UI Tests (require VST3 SDK + VSTGUI) ---
if(NOT POLY_ENGINE_ONLY)
    include(${CMAKE_SOURCE_DIR}/cmake/jk_ui_test_harness.cmake)

    set(POLY_PLUGIN_SOURCES
        ${CMAKE_SOURCE_DIR}/plugin/source/controller.h
        ${CMAKE_SOURCE_DIR}/plugin/source/controller.cpp
        ${CMAKE_SOURCE_DIR}/plugin/source/plugids.h
        ${CMAKE_SOURCE_DIR}/plugin/source/ui/lane_grid_view.h
        ${CMAKE_SOURCE_DIR}/plugin/source/ui/lane_grid_view.cpp
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
