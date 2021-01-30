#include <iostream>
#include <unordered_map>

#include "interpreter/lexer/lexeme.hpp"

namespace interpreter::lexer {

std::ostream& operator<<(std::ostream& out, const Lexeme& lexeme) {
  static const std::unordered_map<LexType, std::string> words = {
      {LexType::AND, "and"},
      {LexType::ASSIGN, "="},
      {LexType::BREAK, "break"},
      {LexType::CASE, "case"},
      {LexType::CLOSING_BRACE, "}"},
      {LexType::CLOSING_PARENTHESIS, ")"},
      {LexType::COMMA, ","},
      {LexType::CONTINUE, "continue"},
      {LexType::DIV, "/"},
      {LexType::DO, "do"},
      {LexType::ELSE, "else"},
      {LexType::END, "end"},
      {LexType::EQ, "=="},
      {LexType::FALSE, "false"},
      {LexType::FOR, "for"},
      {LexType::GE, ">="},
      {LexType::GT, ">"},
      {LexType::IF, "if"},
      {LexType::LE, "<="},
      {LexType::LT, "<"},
      {LexType::MINUS, "-"},
      {LexType::MOD, "%"},
      {LexType::MUL, "*"},
      {LexType::NE, "!="},
      {LexType::NOT, "not"},
      {LexType::NONE, "<none>"},
      {LexType::OF, "of"},
      {LexType::OPENING_BRACE, "{"},
      {LexType::OPENING_PARENTHESIS, "("},
      {LexType::OR, "or"},
      {LexType::PLUS, "+"},
      {LexType::PROGRAM, "program"},
      {LexType::READ, "read"},
      {LexType::SEMICOLON, ";"},
      {LexType::TYPE_BOOL, "boolean"},
      {LexType::TYPE_INT, "int"},
      {LexType::TYPE_REAL, "real"},
      {LexType::TYPE_STR, "string"},
      {LexType::TRUE, "true"},
      {LexType::WHILE, "while"},
      {LexType::WRITE, "write"},
  };

  const auto type = lexeme.type;

  if (words.contains(type)) {
    out << words.at(type);
  } else if (type == LexType::VALUE_STR || type == LexType::VALUE_INT ||
             type == LexType::VALUE_REAL || type == LexType::ID) {
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

}  // namespace interpreter::lexer
