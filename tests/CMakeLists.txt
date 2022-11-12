cmake_minimum_required(VERSION 3.14)
# use doctest as default
find_package(doctest REQUIRED)

# TODO: list all files containg tests
set(TEST_FILES
        # ./coroutine/test_task.cpp
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

add_executable(test_threadpool ./common/test_threadpool.cpp)
set_target_properties(test_threadpool PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
target_link_libraries(test_threadpool PRIVATE ${LIBRARY_NAME} doctest::doctest fmt::fmt)

add_executable(test_coroutine ./coroutine/test_task.cpp)
set_target_properties(test_coroutine PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
target_compile_features(test_coroutine PRIVATE cxx_std_20)
target_link_libraries(test_coroutine PRIVATE ${LIBRARY_NAME} doctest::doctest fmt::fmt)

add_executable(echo_server2 ./net/echo_server2.cpp)
set_target_properties(echo_server2 PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
target_compile_features(echo_server2 PRIVATE cxx_std_20)
target_link_libraries(echo_server2 PRIVATE ${LIBRARY_NAME} doctest::doctest fmt::fmt)

