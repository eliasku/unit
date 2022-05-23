cmake_minimum_required(VERSION 3.19)
project(unit-test C CXX)

add_executable(${PROJECT_NAME}
        main.c
        test_asserts.c
        main.cpp
        )
target_compile_definitions(${PROJECT_NAME} PUBLIC UNIT_TESTING)
target_link_libraries(${PROJECT_NAME} PUBLIC unit)
add_test(NAME ${PROJECT_NAME} COMMAND $<TARGET_FILE:${PROJECT_NAME}>)
test_code_coverage(${PROJECT_NAME})
