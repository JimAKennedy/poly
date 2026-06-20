add_library(poly_engine STATIC
    src/bridge.cpp
    src/engine.cpp
    src/envelope.cpp
    src/euclidean.cpp
    src/macro.cpp
)

target_include_directories(poly_engine
    PUBLIC include
)

jk_target_warnings(poly_engine)
