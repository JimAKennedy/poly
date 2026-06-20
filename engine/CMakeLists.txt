add_library(poly_engine STATIC
    src/engine.cpp
)

target_include_directories(poly_engine
    PUBLIC include
)

jk_target_warnings(poly_engine)
