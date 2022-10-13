# additional target to perform clang-format run, requires clang-format

# get all project files
file(GLOB_RECURSE ALL_SOURCE_FILES *.cpp *.cc *.h *.hpp)
# file (GLOB_RECURSE ALL_SOURCE_FILES
#   src/*.cpp
#   src/*.cc
#   include/*.h
#   include/*.hpp
#   example/*.cpp
#   example/*.cc
#   example/*.h
#   example/*.hpp)

foreach (SOURCE_FILE ${ALL_SOURCE_FILES})
    string(FIND ${SOURCE_FILE} ${PROJECT_TRDPARTY_DIR} PROJECT_TRDPARTY_DIR_FOUND)
    if (NOT ${PROJECT_TRDPARTY_DIR_FOUND} EQUAL -1)
        list(REMOVE_ITEM ALL_SOURCE_FILES ${SOURCE_FILE})
    endif ()
endforeach ()

foreach (SOURCE_FILE ${ALL_SOURCE_FILES})
    string(FIND ${SOURCE_FILE} ${PROJECT_SOURCE_DIR}/build PROJECT_BUILD_DIR_FOUND)
    if (NOT ${PROJECT_BUILD_DIR_FOUND} EQUAL -1)
        list(REMOVE_ITEM ALL_SOURCE_FILES ${SOURCE_FILE})
    endif ()
endforeach ()


add_custom_target(
        clangformat
        COMMAND /usr/bin/clang-format
        -style=file
        -i
        ${ALL_SOURCE_FILES}
)

add_custom_target(
        clangtidy
        COMMAND /usr/bin/clang-tidy
        --config-file=${PROJECT_SOURCE_DIR}/.clang-tidy
        ${ALL_SOURCE_FILES}
)