add_executable(poly_presets_emit
    emit_presets.cpp
)

target_link_libraries(poly_presets_emit PRIVATE poly_engine)

jk_target_warnings(poly_presets_emit)
