cmake_minimum_required(VERSION 3.14)
project(interpreter2)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(interpreter2
        include/interpreter.hpp
        src/interpreter.cpp
        src/main.cpp include/lexer/lexer.hpp include/visitor/visitor.h include/visitor/debug_visitor.h include/visitor/types.h)
