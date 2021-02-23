set (TEST_SOURCES
  lexer/test_lexer.cpp
  ast/test_ast.cpp
  interpreter/test_interpreter.cpp
)

foreach(TEST_FILE_NAME ${TEST_SOURCES})

  target_sources(
    ${PROJECT_NAME}_TEST PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_FILE_NAME}
  )

endforeach()
