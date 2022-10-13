# only for CMake 3.12 or later:
file(GLOB_RECURSE HEADER_LIST CONFIGURE_DEPENDS
        "${PROJECT_SOURCE_DIR}/include/*.h"
        )

# Or you just add header file to HEADER_LIST one by one
# set(HEADER_LIST "${PROJECT_SOURCE_DIR}/include/Hello.hpp")

file(GLOB_RECURSE SOURCE_LIST CONFIGURE_DEPENDS
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
        )

add_library(${LIBRARY_NAME} ${SOURCE_LIST} ${HEADER_LIST})

# TODO: may need to set dependence
set(DEPEND_LIST
        fmt::fmt
        )

find_package(fmt REQUIRED)

target_include_directories(${LIBRARY_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(${LIBRARY_NAME} PRIVATE ${DEPEND_LIST})
target_compile_features(${LIBRARY_NAME} PUBLIC cxx_std_20)

# IDEs should put the headers in a nice place
source_group(
        TREE "${PROJECT_SOURCE_DIR}/include"
        PREFIX "Header Files"
        FILES ${HEADER_LIST}
)