cmake_minimum_required(VERSION 3.19)
project(vector-example C)

add_executable(${PROJECT_NAME}
        vector.c
        test_files.c
        )
target_link_libraries(${PROJECT_NAME} PUBLIC unit)
target_compile_definitions(${PROJECT_NAME} PUBLIC UNIT_TESTING)
add_test(NAME ${PROJECT_NAME} COMMAND ${NODE_JS_EXECUTABLE} $<TARGET_FILE:${PROJECT_NAME}>)
