cmake_minimum_required(VERSION 3.14)
# use doctest as default
find_package(doctest REQUIRED)

# TODO: list all files containg tests
set(TEST_FILES
#        ./coroutine/test_task.cpp
        ./net/echo_server.cpp
        )

# Or you just add all the .cpp file under tests folder
# file(GLOB_RECURSE SOURCE_LIST CONFIGURE_DEPENDS
#   "${PROJECT_SOURCE_DIR}/tests/*.cpp"
# )

# default name for test executable (change if you wish)
set(TEST_MAIN unit_tests)
# any parms to feed the test runner
set(TEST_RUNNER_PARAMS "")

find_package(fmt REQUIRED)

add_executable(${TEST_MAIN} ${TEST_FILES})
target_link_libraries(${TEST_MAIN} PRIVATE ${LIBRARY_NAME} doctest::doctest fmt::fmt)
set_target_properties(${TEST_MAIN} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
target_compile_features(${TEST_MAIN} PRIVATE cxx_std_20)

add_test(
        NAME ${LIBRARY_NAME}.${TEST_MAIN}
        COMMAND ${TEST_MAIN} ${TEST_RUNNER_PARAMS}
)