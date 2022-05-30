cmake_minimum_required(VERSION 3.19)
project(test-unit C CXX)

add_executable(${PROJECT_NAME}
        main.c
        main.cpp
        asserts.c
        fun.c
        )
target_compile_definitions(${PROJECT_NAME} PUBLIC UNIT_TESTING)
target_link_libraries(${PROJECT_NAME} PUBLIC unit)
add_test(NAME ${PROJECT_NAME} COMMAND $<TARGET_FILE:${PROJECT_NAME}>)
add_test(NAME ${PROJECT_NAME}_verbose COMMAND $<TARGET_FILE:${PROJECT_NAME}> --verbose)
test_code_coverage(${PROJECT_NAME})
test_code_coverage(${PROJECT_NAME} NAME ${PROJECT_NAME}_verbose ARGS --verbose)
