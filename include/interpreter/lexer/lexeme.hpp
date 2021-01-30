#pragma once

#include <iosfwd>
#include <string>
#include <variant>

#include "types.hpp"

namespace interpreter::lexer {

struct Lexeme {
  LexType type;
  std::variant<std::monostate, int, double, bool, std::string> data;

  [[nodiscard]] inline constexpr bool operator==(
      const Lexeme& other) const noexcept = default;
};

// TODO: move him in another header
std::ostream& operator<<(std::ostream& out, const Lexeme& lexeme);

}  // namespace interpreter::lexer
