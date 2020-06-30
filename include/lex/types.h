#pragma once

namespace interpreter::lexems {

enum class Type {
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

  // Assign operation

  ASSIGN,

  // arithmetical operations

  _ARITHMETICAL_OPS_START,
  DIV,
  MINUS,
  MOD,
  MUL,
  PLUS,

  // logical operations

  _LOGICAL_OPS_START,
  NOT,
  AND,
  OR,
  LT, // <
  LE, // <=
  GT, // >
  GE, // >=
  EQ, // ==
  NE, // !=
  _LOGICAL_OPS_END,
  _ARITHMETICAL_OPS_END,
};

namespace type_utils {
[[nodiscard]]
constexpr inline bool IsVariableType(Type type) noexcept {
  return Type::_TYPE_START < type && type < Type::_TYPE_END;
}

[[nodiscard]]
constexpr inline bool IsLogicalOperation(Type type) noexcept {
  return Type::_LOGICAL_OPS_START < type && type < Type::_LOGICAL_OPS_END;
}

[[nodiscard]]
constexpr inline bool IsArithmeticalOperation(Type type) noexcept {
  return Type::_ARITHMETICAL_OPS_START < type &&
         type < Type::_ARITHMETICAL_OPS_END;
}

[[nodiscard]]
constexpr inline bool IsConstant(Type type) noexcept {
  return (Type::_VALUES_START < type && type < Type::_VALUES_END) ||
         type == Type::FALSE || type == Type::TRUE;
}

} // namespace type_utils

} // namespace lexems
