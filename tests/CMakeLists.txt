add_executable(${PROJECT_NAME}_TEST)
set_target_properties(${PROJECT_NAME}_TEST PROPERTIES CXX_STANDARD 20)
target_link_libraries(${PROJECT_NAME}_TEST PRIVATE ${PROJECT_NAME}_INCLUDE ${PROJECT_NAME}_LIB)

find_package(Threads)

target_include_directories(
  ${PROJECT_NAME}_TEST PRIVATE
  ${gmock_SOURCE_DIR}/include
)
target_link_libraries(
  ${PROJECT_NAME}_TEST PRIVATE
  ${CMAKE_THREAD_LIBS_INIT}
  gmock_main
)

add_subdirectory(src)

add_test(
  NAME
    ${PROJECT_NAME}
  COMMAND
    ${PROJECT_NAME}
)
