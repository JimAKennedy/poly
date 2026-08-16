smtg_add_vst3plugin(poly_plugin
    source/factory.cpp
    source/processor.h
    source/processor.cpp
    source/controller.h
    source/controller.cpp
    source/controller_base.h
    source/controller_base.cpp
    source/plugids.h
)

target_link_libraries(poly_plugin
    PRIVATE
        poly_engine
        sdk
)

if(APPLE)
    target_sources(poly_plugin PRIVATE
        ${vst3sdk_SOURCE_DIR}/public.sdk/source/main/macmain.cpp
    )
    set_source_files_properties(
        ${vst3sdk_SOURCE_DIR}/public.sdk/source/main/macmain.cpp
        PROPERTIES COMPILE_FLAGS "-Wno-deprecated-declarations"
    )
elseif(WIN32)
    target_sources(poly_plugin PRIVATE
        ${vst3sdk_SOURCE_DIR}/public.sdk/source/main/dllmain.cpp
    )
elseif(UNIX)
    target_sources(poly_plugin PRIVATE
        ${vst3sdk_SOURCE_DIR}/public.sdk/source/main/linuxmain.cpp
    )
endif()

jk_target_warnings(poly_plugin)

# -----------------------------------------------------------------------------
# AudioUnit v2 target — wraps the SAME poly_plugin VST3 as an aumu music device.
# Gated on XCODE + SMTG_ENABLE_AUV2_BUILDS exactly like the SDK's own AU wrapper
# (see cmake/modules/SMTG_AddVST3AuV2.cmake). The root CMakeLists fetches Apple's
# AudioUnitSDK and flips SMTG_ENABLE_AUV2_BUILDS on under the Xcode generator, so
# this never activates in the Ninja VST3/test/pluginval legs.
#
# Identity contract (must match plugin/resource/au-info.plist and the CI auval
# invocation `auval -v aumu Poly JkDl`):
#   type=aumu  subtype=Poly  manufacturer=JkDl  factoryFunction=AUWrapperFactory
# -----------------------------------------------------------------------------
if(APPLE AND XCODE AND SMTG_ENABLE_AUV2_BUILDS)
    # The VST3 SDK appends its cmake/modules to CMAKE_MODULE_PATH inside its own
    # FetchContent subdirectory scope, which does not propagate back to this
    # scope — so include(SMTG_AddVST3AuV2) by bare name is not resolvable here.
    # Re-add the SDK module dir explicitly before the include.
    list(APPEND CMAKE_MODULE_PATH "${vst3sdk_SOURCE_DIR}/cmake/modules")
    # public_sdk_SOURCE_DIR is likewise a plain variable set in the SDK's own
    # subdirectory scope (points at the auwrapper .mm sources the macro compiles);
    # it is empty here, so re-derive it before smtg_target_add_auv2 or the macro
    # creates poly_au with no sources.
    set(public_sdk_SOURCE_DIR "${vst3sdk_SOURCE_DIR}/public.sdk")
    include(SMTG_AddVST3AuV2)
    smtg_target_add_auv2(poly_au
        BUNDLE_NAME Poly
        BUNDLE_IDENTIFIER digital.jk.poly.audiounit
        INFO_PLIST_TEMPLATE ${CMAKE_CURRENT_SOURCE_DIR}/resource/au-info.plist
        VST3_PLUGIN_TARGET poly_plugin)
endif()

# -----------------------------------------------------------------------------
# choc-webview web UI — the shipping DAW editor as of M052 (2026-07-20).
# See webui/README.md for architecture and bridge-schema.md for the protocol.
#
# Platform-gated to macOS+Windows (the shipping plugin targets). Linux is
# deliberately excluded, NOT "excluded until CI is wired up": M054 decision D029
# (Path 3) pins Linux to engine/WASM only — Poly ships no Linux VST3 binary, so
# the Ubuntu CI leg is now an engine-only (-DPOLY_ENGINE_ONLY=ON) portability
# compile that never reaches this predicate. There is no plan to add choc's
# WebKitGTK backend; the native VSTGUI editor was already removed in M053/S05, so
# were a full Linux plugin ever built it would ship no editor (controller.cpp's
# createView returns nullptr). Keeping the `if(APPLE OR WIN32)` guard makes that
# scope explicit at the build boundary.
#
# There is intentionally no build flag here: the platform condition IS the
# selector, so local dev and CI cannot diverge on which editor ships. The same
# condition keys controller.cpp's editor selection via the __APPLE__/_WIN32
# predefined macros (no separate compile definition).
# -----------------------------------------------------------------------------
if(APPLE OR WIN32)
    include(FetchContent)
    FetchContent_Declare(choc
        GIT_REPOSITORY https://github.com/Tracktion/choc.git
        GIT_TAG 5685fb59db9e60d31a35df7bff0ce7967beaaf25
    )
    FetchContent_MakeAvailable(choc)

    include(${CMAKE_SOURCE_DIR}/cmake/jk_embed_assets.cmake)
    jk_embed_assets(
        ${CMAKE_CURRENT_BINARY_DIR}/generated/poly_webui_assets.h
        webui_assets
        ${CMAKE_SOURCE_DIR}/webui/index.html
        ${CMAKE_SOURCE_DIR}/webui/ui.css
        ${CMAKE_SOURCE_DIR}/webui/ui.js
        ${CMAKE_SOURCE_DIR}/webui/host-iface.js
        ${CMAKE_SOURCE_DIR}/webui/groove-math.js
        ${CMAKE_SOURCE_DIR}/webui/mock-host.js
        ${CMAKE_SOURCE_DIR}/webui/plugin-host.js
    )

    target_sources(poly_plugin PRIVATE
        source/webui/web_ui_view.h
        source/webui/web_ui_view.cpp
        source/webui/bridge_params.h
        source/webui/bridge_serialization.h
        source/webui/bridge_serialization.cpp
        source/webui/platform_save_dialog.h
        source/webui/platform_drag_source.h
    )
    if(APPLE)
        target_sources(poly_plugin PRIVATE
            source/webui/platform_save_dialog_mac.mm
            source/webui/platform_drag_source_mac.mm)
        target_link_libraries(poly_plugin PRIVATE "-framework AppKit" "-framework Foundation")
    elseif(WIN32)
        target_sources(poly_plugin PRIVATE
            source/webui/platform_save_dialog_win.cpp
            source/webui/platform_drag_source_win.cpp)
        target_link_libraries(poly_plugin PRIVATE ole32 shell32 shlwapi)
    endif()
    target_include_directories(poly_plugin PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source/webui
        ${choc_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/generated
    )
endif()
