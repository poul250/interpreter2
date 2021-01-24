add_executable(${PROJECT_NAME}_TEST)
set_target_properties(${PROJECT_NAME}_TEST PROPERTIES CXX_STANDARD 20)
target_link_libraries(${PROJECT_NAME}_TEST PRIVATE ${PROJECT_NAME}_INCLUDE ${PROJECT_NAME}_LIB)

find_package(GTest REQUIRED)
target_include_directories(${PROJECT_NAME}_TEST PRIVATE ${GTEST_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}_TEST PRIVATE ${GTEST_BOTH_LIBRARIES} pthread)

add_subdirectory(src)

add_test(
  NAME
    ${PROJECT_NAME}
  COMMAND
    ${PROJECT_NAME}
)
