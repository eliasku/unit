cmake_minimum_required(VERSION 3.19)
project(test-unit C CXX)

set(SOURCE_FILES main.c
        main.cpp
        asserts.c
        fun.c)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_compile_definitions(${PROJECT_NAME} PUBLIC UNIT_TESTING)
target_link_libraries(${PROJECT_NAME} PUBLIC unit)
add_test(NAME ${PROJECT_NAME} COMMAND ${NODE_JS_EXECUTABLE} $<TARGET_FILE:${PROJECT_NAME}>)
test_code_coverage(${PROJECT_NAME})

project(test-unit-disabled C CXX)
add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} PUBLIC unit)
