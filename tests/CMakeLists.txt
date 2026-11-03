add_executable(tuple_tests
    tuple_tests.cpp
    ../src/Example.cpp
)
target_include_directories(tuple_tests PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(tuple_tests PRIVATE GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(tuple_tests)