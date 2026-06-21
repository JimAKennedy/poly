# =============================================================================
# jk_ui_test_harness.cmake — Shared VSTGUI UI test infrastructure
# Canonical source: audio-meta/test_harness/
#
# Provides two functions:
#   jk_add_visual_tests(target)      — offscreen rendering + pixel comparison
#   jk_add_interaction_tests(target) — headless UI host + input simulation
# =============================================================================

# -----------------------------------------------------------------------------
# jk_add_visual_tests(target)
#
# Required CMake variables (set before calling):
#   VISUAL_TEST_HARNESS_DIR  — path to the directory containing the harness files
#
# Optional CMake variables:
#   VISUAL_TEST_REFERENCE_DIR — path to reference images
#   VISUAL_TEST_OUTPUT_DIR    — path for test output
# -----------------------------------------------------------------------------
function(jk_add_visual_tests target)
    if(NOT DEFINED VISUAL_TEST_HARNESS_DIR)
        message(FATAL_ERROR "VISUAL_TEST_HARNESS_DIR must be set before calling jk_add_visual_tests()")
    endif()

    if(NOT DEFINED VISUAL_TEST_REFERENCE_DIR)
        set(VISUAL_TEST_REFERENCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/references")
    endif()
    if(NOT DEFINED VISUAL_TEST_OUTPUT_DIR)
        set(VISUAL_TEST_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/visual_output")
    endif()

    file(MAKE_DIRECTORY ${VISUAL_TEST_REFERENCE_DIR})
    file(MAKE_DIRECTORY ${VISUAL_TEST_OUTPUT_DIR})

    target_sources(${target} PRIVATE
        ${VISUAL_TEST_HARNESS_DIR}/visual_test_harness.h
        ${VISUAL_TEST_HARNESS_DIR}/visual_test_harness.cpp
        ${VISUAL_TEST_HARNESS_DIR}/image_compare.h
        ${VISUAL_TEST_HARNESS_DIR}/image_compare.cpp
    )

    target_include_directories(${target} PRIVATE
        ${VISUAL_TEST_HARNESS_DIR}
    )

    target_compile_definitions(${target} PRIVATE
        VISUAL_TEST_REFERENCE_DIR="${VISUAL_TEST_REFERENCE_DIR}"
        VISUAL_TEST_OUTPUT_DIR="${VISUAL_TEST_OUTPUT_DIR}"
    )

    if(APPLE)
        target_link_libraries(${target} PRIVATE
            "-framework CoreFoundation"
            "-framework CoreGraphics"
            "-framework ImageIO"
        )
    endif()
endfunction()

# -----------------------------------------------------------------------------
# jk_add_interaction_tests(target)
#
# Required CMake variables (set before calling):
#   INTERACTION_TEST_HARNESS_DIR — path to the directory containing
#                                  headless_ui_host.h/cpp
#   (Falls back to VISUAL_TEST_HARNESS_DIR if not set)
# -----------------------------------------------------------------------------
function(jk_add_interaction_tests target)
    if(NOT DEFINED INTERACTION_TEST_HARNESS_DIR)
        if(DEFINED VISUAL_TEST_HARNESS_DIR)
            set(INTERACTION_TEST_HARNESS_DIR ${VISUAL_TEST_HARNESS_DIR})
        else()
            message(FATAL_ERROR "INTERACTION_TEST_HARNESS_DIR must be set before calling jk_add_interaction_tests()")
        endif()
    endif()

    target_sources(${target} PRIVATE
        ${INTERACTION_TEST_HARNESS_DIR}/headless_ui_host.h
        ${INTERACTION_TEST_HARNESS_DIR}/headless_ui_host.cpp
    )

    target_include_directories(${target} PRIVATE
        ${INTERACTION_TEST_HARNESS_DIR}
    )

    if(APPLE)
        target_link_libraries(${target} PRIVATE
            "-framework CoreFoundation"
            "-framework CoreGraphics"
            "-framework ImageIO"
            "-framework AppKit"
        )

        set_target_properties(${target} PROPERTIES
            LINK_FLAGS "-Wl,-undefined,dynamic_lookup"
        )
    endif()
endfunction()
