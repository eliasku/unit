cmake_minimum_required(VERSION 3.19)
include(cmake/code-coverage.cmake)

project(unit C)
add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE include)
target_code_coverage(${PROJECT_NAME} INTERFACE)

if ($ENV{CLION_IDE})
    set(UNIT_TESTING ON)
endif()

if (UNIT_TESTING)
    enable_testing()

    project(unit-test C)
    add_executable(${PROJECT_NAME}
            test/main.c
            test/test_files.c
            test/test_asserts.c
            )
    target_link_libraries(${PROJECT_NAME} PUBLIC unit)
    add_test(NAME ${PROJECT_NAME} COMMAND $<TARGET_FILE:${PROJECT_NAME}>)
    test_code_coverage(${PROJECT_NAME})

    project(vector-example C)
    add_executable(${PROJECT_NAME} example/vector.c)
    target_link_libraries(${PROJECT_NAME} PUBLIC unit)
    add_test(NAME ${PROJECT_NAME} COMMAND $<TARGET_FILE:${PROJECT_NAME}>)
    set_tests_properties(${PROJECT_NAME} PROPERTIES WILL_FAIL TRUE)
    target_code_coverage(${PROJECT_NAME} PUBLIC)
    test_code_coverage(${PROJECT_NAME})

endif ()

coverage_all()