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
