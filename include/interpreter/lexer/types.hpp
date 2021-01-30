#pragma once

namespace interpreter::lexer {

enum class LexType {
  NONE = 0,
  ID,

  _VALUES_START,
  VALUE_INT,
  VALUE_STR,
  VALUE_REAL,
  _VALUES_END,

  // Key words

  BREAK,
  CASE,
  CONTINUE,
  DO,
  ELSE,
  END,
  FALSE,
  FOR,
  IF,
  OF,
  PROGRAM,
  READ,
  TRUE,
  WHILE,
  WRITE,

  // Type words

  _TYPE_START,

  TYPE_INT,
  TYPE_STR,
  TYPE_REAL,
  TYPE_BOOL,

  _TYPE_END,

  // Brackets

  OPENING_BRACE,
  CLOSING_BRACE,
  OPENING_PARENTHESIS,
  CLOSING_PARENTHESIS,
  SEMICOLON,
  COMMA,

  _ARITHMETICAL_OPS_START,
  ASSIGN,
  PLUS,
  MINUS,
  DIV,
  MOD,
  MUL,
  _LOGICAL_OPS_START,
  NOT,
  AND,
  OR,
  _COMPARE_OPS_START,
  LT,  // <
  LE,  // <=
  GT,  // >
  GE,  // >=
  EQ,  // ==
  NE,  // !=
  _COMPARE_OPS_END,
  _LOGICAL_OPS_END,
  _ARITHMETICAL_OPS_END,
};

[[nodiscard]] constexpr bool IsVariableType(LexType type) noexcept {
  return LexType::_TYPE_START < type && type < LexType::_TYPE_END;
}

[[nodiscard]] constexpr bool IsLogicalOperation(LexType type) noexcept {
  return LexType::_LOGICAL_OPS_START < type && type < LexType::_LOGICAL_OPS_END;
}

[[nodiscard]] constexpr bool IsArithmeticalOperation(LexType type) noexcept {
  return LexType::_ARITHMETICAL_OPS_START < type &&
         type < LexType::_ARITHMETICAL_OPS_END;
}

[[nodiscard]] constexpr bool IsConstant(LexType type) noexcept {
  return (LexType::_VALUES_START < type && type < LexType::_VALUES_END) ||
         type == LexType::FALSE || type == LexType::TRUE;
}

[[nodiscard]] constexpr bool IsBoolean(LexType type) noexcept {
  return type == LexType::FALSE || type == LexType::TRUE;
}

[[nodiscard]] constexpr bool IsCompare(LexType type) noexcept {
  return LexType::_COMPARE_OPS_START < type && type < LexType::_COMPARE_OPS_END;
}

}  // namespace interpreter::lexer
