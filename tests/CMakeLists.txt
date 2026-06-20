include(FetchContent)
FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(poly_tests
    euclidean_tests.cpp
    golden_tests.cpp
    dynamic_shaping_tests.cpp
)

target_link_libraries(poly_tests
    PRIVATE poly_engine GTest::gtest_main
)

jk_target_warnings(poly_tests)

include(GoogleTest)
gtest_discover_tests(poly_tests)
