cmake_minimum_required(VERSION 3.0.0)

project(
  interpreter2 
  VERSION 0.0.1
  LANGUAGES CXX
)

include(cmake/StandardSettings.cmake)

# Prevent building in the source directory
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.\n")
endif()

# Add include interface library
add_library(${PROJECT_NAME}_INCLUDE INTERFACE)
target_include_directories(${PROJECT_NAME}_INCLUDE INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Loop holes are not warnings ;)
if (NOT MSVC)
  target_compile_options(${PROJECT_NAME}_INCLUDE INTERFACE "-Wno-non-template-friend")
endif()

# interpreter library
add_subdirectory(lib)

# interpreter cli if needed
if (${PROJECT_NAME}_BUILD_EXECUTABLE)
  add_subdirectory(src)
endif()

# build tests if needed
if (${PROJECT_NAME}_ENABLE_UNIT_TESTING)
  add_subdirectory(thirdparty/googletest)
  enable_testing()
  add_subdirectory(tests)
endif()
