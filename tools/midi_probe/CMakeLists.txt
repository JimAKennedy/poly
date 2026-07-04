# Probe is a test-only tool — skip the VST3 validator (no controller class)
set(_SAVED_VALIDATOR ${SMTG_RUN_VST_VALIDATOR})
set(SMTG_RUN_VST_VALIDATOR OFF)

smtg_add_vst3plugin(poly_midi_probe
    source/probe_factory.cpp
    source/probe_processor.h
    source/probe_processor.cpp
    source/probe_ids.h
)

set(SMTG_RUN_VST_VALIDATOR ${_SAVED_VALIDATOR})

target_link_libraries(poly_midi_probe
    PRIVATE
        sdk
)

if(APPLE)
    target_sources(poly_midi_probe PRIVATE
        ${vst3sdk_SOURCE_DIR}/public.sdk/source/main/macmain.cpp
    )
    set_source_files_properties(
        ${vst3sdk_SOURCE_DIR}/public.sdk/source/main/macmain.cpp
        PROPERTIES COMPILE_FLAGS "-Wno-deprecated-declarations"
    )
elseif(WIN32)
    target_sources(poly_midi_probe PRIVATE
        ${vst3sdk_SOURCE_DIR}/public.sdk/source/main/dllmain.cpp
    )
elseif(UNIX)
    target_sources(poly_midi_probe PRIVATE
        ${vst3sdk_SOURCE_DIR}/public.sdk/source/main/linuxmain.cpp
    )
endif()

jk_target_warnings(poly_midi_probe)
