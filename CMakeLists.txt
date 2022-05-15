cmake_minimum_required(VERSION 3.19)
project(unit C)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE include)

if (UNIT_TESTING OR $ENV{CLION_IDE})
    enable_testing()

    project(unit-test C)
    add_executable(${PROJECT_NAME} test/main.c)
    target_link_libraries(${PROJECT_NAME} PUBLIC unit)
    add_test(NAME ${PROJECT_NAME} COMMAND $<TARGET_FILE:${PROJECT_NAME}>)

    project(vector-example C)
    add_executable(${PROJECT_NAME} example/vector.c)
    target_link_libraries(${PROJECT_NAME} PUBLIC unit)
    add_test(NAME ${PROJECT_NAME} COMMAND $<TARGET_FILE:${PROJECT_NAME}>)
    set_tests_properties(${PROJECT_NAME} PROPERTIES WILL_FAIL TRUE)

endif ()
