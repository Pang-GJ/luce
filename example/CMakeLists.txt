cmake_minimum_required(VERSION 3.20)

add_executable(echo_server ./echo_server.cpp)
set_target_properties(echo_server PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
target_compile_features(echo_server PRIVATE cxx_std_20)
target_link_libraries(echo_server PRIVATE ${LIBRARY_NAME})