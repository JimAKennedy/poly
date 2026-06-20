add_library(poly_engine STATIC
    src/engine.cpp
    src/envelope.cpp
    src/euclidean.cpp
)

target_include_directories(poly_engine
    PUBLIC include
)

jk_target_warnings(poly_engine)
