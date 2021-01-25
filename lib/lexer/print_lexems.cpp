#include <iostream>
#include <unordered_map>

#include "interpreter/lexer/lexems.hpp"

namespace interpreter::lexems {

std::ostream& operator<<(std::ostream& out, const Lexeme& lexeme) {
  static const std::unordered_map<Type, std::string> words = {
      {Type::AND, "and"},
      {Type::ASSIGN, "="},
      {Type::BREAK, "break"},
      {Type::CASE, "case"},
      {Type::CLOSING_BRACE, "}"},
      {Type::CLOSING_PARENTHESIS, ")"},
      {Type::COMMA, ","},
      {Type::CONTINUE, "continue"},
      {Type::DIV, "/"},
      {Type::DO, "do"},
      {Type::ELSE, "else"},
      {Type::END, "end"},
      {Type::EQ, "=="},
      {Type::FALSE, "false"},
      {Type::FOR, "for"},
      {Type::GE, ">="},
      {Type::GT, ">"},
      {Type::IF, "if"},
      {Type::LE, "<="},
      {Type::LT, "<"},
      {Type::MINUS, "-"},
      {Type::MOD, "%"},
      {Type::MUL, "*"},
      {Type::NE, "!="},
      {Type::NOT, "not"},
      {Type::NONE, "<none>"},
      {Type::OF, "of"},
      {Type::OPENING_BRACE, "{"},
      {Type::OPENING_PARENTHESIS, "("},
      {Type::OR, "or"},
      {Type::PLUS, "+"},
      {Type::PROGRAM, "program"},
      {Type::READ, "read"},
      {Type::SEMICOLON, ";"},
      {Type::TYPE_BOOL, "boolean"},
      {Type::TYPE_INT, "int"},
      {Type::TYPE_REAL, "real"},
      {Type::TYPE_STR, "string"},
      {Type::TRUE, "true"},
      {Type::WHILE, "while"},
      {Type::WRITE, "write"},
  };

  const auto type = lexeme.type;

  if (words.contains(type)) {
    out << words.at(type);
  } else if (type == Type::VALUE_STR || type == Type::VALUE_INT ||
             type == Type::VALUE_REAL || type == Type::ID) {
    std::visit(
        [&out](const auto& val) {
          if constexpr (!std::is_same_v<std::decay_t<decltype(val)>,
                                        std::monostate>) {
            out << val;
          } else {
            std::cout << "ERROR, EMPTY LEXEME";
          }
        },
        lexeme.data);
  }

  return out;
}

}  // namespace interpreter::lexems
