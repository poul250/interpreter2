cmake_policy(SET CMP0076 NEW)

target_sources(
  ${PROJECT_NAME}_LIB PRIVATE
  instructions.cpp
  writer.cpp
)
