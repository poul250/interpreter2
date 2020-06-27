#pragma once

namespace interpreter::lex {

enum class Type {
  NONE = 0,
  ID,
  VALUE_INT,
  VALUE_STR,
  VALUE_REAL,

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
  TYPE_INT,
  TYPE_STR,
  TYPE_REAL,
  TYPE_BOOL,

  // Brackets
  OPENING_BRACE,
  CLOSING_BRACE,
  OPENING_PARENTHESIS,
  CLOSING_PARENTHESIS,

  // Assign operation
  ASSIGN,

  // arithmetical operations
  PLUS,
  MINUS,
  DIV,
  MOD,
  MUL,

  // logical operations
  NOT,
  AND,
  OR,
  LT, // <
  LE, // <=
  GT, // >
  GE, // >=
  EQ, // ==
  NE, // !=
};

} // namespace lex;
