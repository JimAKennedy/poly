smtg_add_vst3plugin(poly_plugin
    source/factory.cpp
    source/processor.h
    source/processor.cpp
    source/controller.h
    source/controller.cpp
    source/controller_base.h
    source/controller_base.cpp
    source/plugids.h
    source/ui/cell_editor_view.h
    source/ui/cell_editor_view.cpp
    source/ui/cross_rhythm_view.h
    source/ui/cross_rhythm_view.cpp
    source/ui/envelope_curve_view.h
    source/ui/envelope_curve_view.cpp
    source/ui/header_view.h
    source/ui/header_view.cpp
    source/ui/lane_edit_view.h
    source/ui/lane_edit_view.cpp
    source/ui/lane_grid_view.h
    source/ui/lane_grid_view.cpp
    source/ui/phase_alignment_view.h
    source/ui/phase_alignment_view.cpp
    source/ui/chain_popover_view.h
    source/ui/chain_popover_view.cpp
    source/ui/scene_bar_view.h
    source/ui/scene_bar_view.cpp
    source/ui/micro_timing_editor_view.h
    source/ui/micro_timing_editor_view.cpp
    source/ui/note_map_view.h
    source/ui/note_map_view.cpp
    source/ui/timeline_step_editor_view.h
    source/ui/timeline_step_editor_view.cpp
    source/ui/velocity_view.h
    source/ui/velocity_view.cpp
    source/ui/export_controls_view.h
    source/ui/export_controls_view.cpp
)

smtg_target_add_plugin_resources(poly_plugin
    RESOURCES
        resource/poly.uidesc
)

target_link_libraries(poly_plugin
    PRIVATE
        poly_engine
        sdk
        vstgui_support
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
# choc-webview web UI — the shipping DAW editor as of M052 (2026-07-20).
# Flip to OFF to build the legacy native VSTGUI editor from plugin/source/ui/*;
# M053 will decommission that path entirely after a full feature-parity audit.
# See webui/README.md for architecture and bridge-schema.md for the protocol.
# -----------------------------------------------------------------------------
option(POLY_WEB_UI "Build the choc-webview web UI editor (shipping default)" ON)
if(POLY_WEB_UI)
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
    )
    target_include_directories(poly_plugin PRIVATE
        ${choc_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/generated
    )
    target_compile_definitions(poly_plugin PRIVATE POLY_WEB_UI=1)
endif()
