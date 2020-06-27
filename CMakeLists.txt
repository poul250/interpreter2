cmake_minimum_required(VERSION 3.14)
project(interpreter2)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)

add_executable(interpreter2
        include/interpreter.hpp
        src/interpreter.cpp
        src/main.cpp include/visitor/visitor.h include/lex/types.h include/lex/lexeme.h src/lex/lexeme.cpp)
