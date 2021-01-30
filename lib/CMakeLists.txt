cmake_minimum_required(VERSION 3.10.0)

cmake_policy(SET CMP0076 NEW)

add_library(${PROJECT_NAME}_LIB STATIC)
set_target_properties(${PROJECT_NAME}_LIB PROPERTIES CXX_STANDARD 20)
target_link_libraries(${PROJECT_NAME}_LIB PRIVATE ${PROJECT_NAME}_INCLUDE)
target_compile_options(${PROJECT_NAME}_LIB PUBLIC -fcoroutines)

add_subdirectory(lexer)
add_subdirectory(ast)
add_subdirectory(instructions)
