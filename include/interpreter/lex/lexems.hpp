#pragma once

#include <iosfwd>
#include <stdexcept>
#include <string>
#include <variant>

#include "interpreter/utils/generator.hpp"
#include "types.hpp"

namespace interpreter::lexems {

struct LexicalError : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

struct Lexeme {
  Type type;
  std::variant<std::monostate, int, double, bool, std::string> data;

  [[nodiscard]] inline constexpr bool operator==(
      const Lexeme& other) const noexcept = default;
};

// TODO: make template<typename CharT> for non char strings
utils::generator<Lexeme> ParseLexems(std::istream& input);

// TODO: move him in another header
std::ostream& operator<<(std::ostream& out, const Lexeme& lexeme);

}  // namespace interpreter::lexems
