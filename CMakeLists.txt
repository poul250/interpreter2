cmake_minimum_required(VERSION 3.14)
project(interpreter2)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)

add_executable(interpreter2
        include/interpreter.hpp
        src/interpreter.cpp
        src/main.cpp include/lex/types.h include/lex/lexems.h src/lex/lexems.cpp include/syntax/visitor.h src/lex/print_lexems.cpp src/syntax/visitor.cpp)
