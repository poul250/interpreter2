#pragma once

#include "interpreter/utils/generator.hpp"

#include "lexeme.hpp"

namespace interpreter::lexer {

struct LexicalError : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

// TODO: make template<typename CharT> for non char strings
utils::generator<Lexeme> ParseLexems(std::istream& input);

}  // namespace interpreter::lexer
