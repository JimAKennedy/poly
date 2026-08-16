add_executable(poly_presets_emit
    emit_presets.cpp
)

target_link_libraries(poly_presets_emit PRIVATE poly_engine)

jk_target_warnings(poly_presets_emit)

add_executable(poly_params_emit
    emit_params.cpp
)

target_link_libraries(poly_params_emit PRIVATE poly_engine)

jk_target_warnings(poly_params_emit)

# Offline SMF emitter — writes a deterministic Format-1 .mid from the same
# renderPatternToSMF primitive the WebUI Export chip uses, so the independent
# SMF validator (tests/cubase/validate_smf_export.py) has a real engine-produced
# file to check in normal CI without Cubase (M032 export UAT automation).
add_executable(poly_smf_emit
    emit_smf.cpp
)

target_link_libraries(poly_smf_emit PRIVATE poly_engine)

jk_target_warnings(poly_smf_emit)
