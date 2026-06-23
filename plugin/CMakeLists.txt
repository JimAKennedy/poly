smtg_add_vst3plugin(poly_plugin
    source/factory.cpp
    source/processor.h
    source/processor.cpp
    source/controller.h
    source/controller.cpp
    source/plugids.h
    source/ui/cell_editor_view.h
    source/ui/cell_editor_view.cpp
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
    source/ui/micro_timing_editor_view.h
    source/ui/micro_timing_editor_view.cpp
    source/ui/timeline_step_editor_view.h
    source/ui/timeline_step_editor_view.cpp
    source/ui/velocity_view.h
    source/ui/velocity_view.cpp
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
