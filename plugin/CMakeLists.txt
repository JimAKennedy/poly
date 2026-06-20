smtg_add_vst3plugin(poly_plugin
    source/factory.cpp
    source/processor.h
    source/processor.cpp
    source/controller.h
    source/controller.cpp
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
