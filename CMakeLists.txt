cmake_minimum_required(VERSION 3.19)

project(unit C)
add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE include)

# alias for NPM library name
add_library(ekx::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# STANDALONE project
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(UNIT_LIBRARY_PROJECT ON)
endif ()

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fsanitize=address -fsanitize=undefined -fsanitize=bounds -fsanitize=enum -fsanitize=signed-integer-overflow -fsanitize=null -fsanitize=unsigned-integer-overflow -fsanitize=return -fsanitize=integer-divide-by-zero -fsanitize=unreachable -fsanitize=alignment")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -g -fsanitize=address -fsanitize=undefined -fsanitize=bounds -fsanitize=enum -fsanitize=signed-integer-overflow -fsanitize=null -fsanitize=unsigned-integer-overflow -fsanitize=return -fsanitize=integer-divide-by-zero -fsanitize=unreachable -fsanitize=alignment")

if (UNIT_LIBRARY_PROJECT)
    enable_testing()

    include(cmake/code-coverage.cmake)
    target_code_coverage(unit INTERFACE)

    add_subdirectory(test)
    add_subdirectory(example)

    coverage_all()
endif ()
