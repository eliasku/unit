cmake_minimum_required(VERSION 3.19)
project(test-fail C)

add_executable(${PROJECT_NAME} main.c)
target_compile_definitions(${PROJECT_NAME} PUBLIC UNIT_TESTING)
target_link_libraries(${PROJECT_NAME} PUBLIC unit)
add_test(NAME ${PROJECT_NAME} COMMAND ${NODE_JS_EXECUTABLE} $<TARGET_FILE:${PROJECT_NAME}>)
# this test should fail
set_tests_properties(${PROJECT_NAME} PROPERTIES WILL_FAIL TRUE)
test_code_coverage(${PROJECT_NAME})
