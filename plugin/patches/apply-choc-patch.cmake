# M042 S09: idempotent PATCH_COMMAND helper for the choc WebView2 CDP patch.
#
# FetchContent runs PATCH_COMMAND directly (no shell), so `||` / `2>/dev/null`
# would be passed to git as literal args rather than interpreted. This script
# gives us shell-free idempotency: if the patch is already applied (warm cache),
# `git apply --reverse --check` succeeds and we skip; otherwise we apply it.
#
# Invoked as:
#   ${CMAKE_COMMAND} -DPATCH=<abs .patch> -DREPO=<choc src dir> -P apply-choc-patch.cmake

if(NOT DEFINED PATCH OR NOT DEFINED REPO)
    message(FATAL_ERROR "apply-choc-patch: PATCH and REPO must be set")
endif()

find_program(GIT_EXECUTABLE git REQUIRED)

# Already applied? --reverse --check succeeds only when the patch is present.
execute_process(
    COMMAND "${GIT_EXECUTABLE}" apply --reverse --check "${PATCH}"
    WORKING_DIRECTORY "${REPO}"
    RESULT_VARIABLE already_applied
    OUTPUT_QUIET ERROR_QUIET)

if(already_applied EQUAL 0)
    message(STATUS "choc CDP patch already applied — skipping")
    return()
endif()

execute_process(
    COMMAND "${GIT_EXECUTABLE}" apply "${PATCH}"
    WORKING_DIRECTORY "${REPO}"
    RESULT_VARIABLE apply_result
    OUTPUT_VARIABLE apply_out
    ERROR_VARIABLE apply_err)

if(NOT apply_result EQUAL 0)
    message(FATAL_ERROR
        "choc CDP patch failed to apply (git apply exit ${apply_result}).\n"
        "The pinned choc revision may have changed — re-verify "
        "plugin/patches/choc-webview2-cdp-args.patch.\n${apply_out}${apply_err}")
endif()

message(STATUS "choc CDP patch applied")
