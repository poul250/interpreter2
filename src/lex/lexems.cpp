#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include <lex/lexems.h>

namespace interpreter::lexems {
namespace {

inline bool isliteral(char ch) {
  return isdigit(ch) || isalpha(ch) || ch == '_';
}

const std::unordered_map<std::string, Type> STRING_TO_TYPE = {
        {"and",      Type::AND},
        {"boolean",  Type::TYPE_BOOL},
        {"break",    Type::BREAK},
        {"case",     Type::CASE},
        {"continue", Type::CONTINUE},
        {"do",       Type::DO},
        {"else",     Type::ELSE},
        {"end",      Type::END},
        {"false",    Type::FALSE},
        {"for",      Type::FOR},
        {"if",       Type::IF},
        {"int",      Type::TYPE_INT},
        {"not",      Type::NOT},
        {"of",       Type::OF},
        {"or",       Type::OR},
        {"program",  Type::PROGRAM},
        {"read",     Type::READ},
        {"real",     Type::TYPE_REAL},
        {"string",   Type::TYPE_STR},
        {"true",     Type::TRUE},
        {"while",    Type::WHILE},
        {"write",    Type::WRITE},

        {"!=",       Type::NE},
        {"==",       Type::EQ},
        {"<=",       Type::LE},
        {">=",       Type::GE},
};

const std::unordered_set<char> COMPLEX_CHARS = {
        '!', '=', '<', '>', '/'
};

const std::unordered_map<char, Type> SINGLE_CHAR = {
        {'=', Type::ASSIGN},
        {'<', Type::LT},
        {'>', Type::GT},
        {'/', Type::DIV},
        {'+', Type::PLUS},
        {'-', Type::MINUS},
        {'%', Type::MOD},
        {'*', Type::MUL},
        {';', Type::SEMICOLON},
        {',', Type::COMMA},
        {'{', Type::OPENING_BRACE},
        {'}', Type::CLOSING_BRACE},
        {'(', Type::OPENING_PARENTHESIS},
        {')', Type::CLOSING_PARENTHESIS},
};

template <typename Key, typename Value>
std::unordered_map<Value, Key>
ReverseMap(const std::unordered_map<Key, Value> map) {
  std::unordered_map<Value, Key> result;
  for (const auto&[key, value] : map) {
    result.emplace(value, key);
  }
  return result;
}

} // namespcae

Lexer::Lexer(std::istream& input, int line)
        : _input(input), _line(line), _current_state{&Lexer::Idle} { }

Lexeme Lexer::GetNext() {
  if (_input.eof()) return {};

  _buf.clear();

  char ch;
  while (_input.get(ch)) {
    const auto&[next_state, unget, opt_lexeme] = _current_state(
            *this, ch, false);

    _prev_ch = ch;
    _current_state = next_state;

    if (unget) {
      _input.unget();
    } else {
      _line += ch == '\n';
    }
    if (opt_lexeme) return *opt_lexeme;
  }

  const auto&[_, __, opt_lexeme] = _current_state(*this, ch, true);
  return opt_lexeme.value_or(Lexeme{});
}

Lexer::StateResult Lexer::Idle(char ch, bool eof) {
  if (eof || isspace(ch)) {
    return {&Lexer::Idle};
  }

  if (isalpha(ch)) {
    return {&Lexer::ReadWord, true};
  }

  if (isdigit(ch)) {
    return {&Lexer::ReadInteger, true};
  }

  if (ch == '"') {
    return {&Lexer::ReadString};
  }

  if (COMPLEX_CHARS.count(ch) > 0) {
    return {&Lexer::ComplexOp};
  }

  if (auto it = SINGLE_CHAR.find(ch); it != SINGLE_CHAR.end()) {
    return {&Lexer::Idle, false, Lexeme{it->second}};
  }

  throw LexicalError(std::string{"Unrecognized symbol "} + ch);
}

Lexer::StateResult Lexer::ComplexOp(char ch, bool eof) {
  if (eof) {
    if (auto it = SINGLE_CHAR.find(_prev_ch); it != SINGLE_CHAR.end()) {
      return {&Lexer::Idle, false, Lexeme{it->second}};
    }
    throw LexicalError{"Unexpected end of file"};
  }

  if (_prev_ch == '/') {
    if (ch == '/') return {&Lexer::LineComment};
    if (ch == '*') return {&Lexer::ManyLineCommentStar};

    return {&Lexer::Idle, true, Lexeme{Type::DIV}};
  }

  std::string complex_op = std::string{_prev_ch} + ch;
  if (auto it = STRING_TO_TYPE.find(complex_op); it != STRING_TO_TYPE.end()) {
    return {&Lexer::Idle, false, Lexeme{it->second}};
  }

  if (auto it = SINGLE_CHAR.find(_prev_ch); it != SINGLE_CHAR.end()) {
    return {&Lexer::Idle, true, Lexeme{it->second}};
  }

  throw LexicalError{std::string{"Unexpected symbol"} + _prev_ch};
}

Lexer::StateResult Lexer::ReadWord(char ch, bool eof) {
  if (eof || !isalpha(ch)) {
    auto it = STRING_TO_TYPE.find(_buf);
    if (it != STRING_TO_TYPE.end()) {
      return {&Lexer::Idle, !eof, Lexeme{it->second}};
    }
    return {&Lexer::Idle, !eof, Lexeme{Type::ID, std::move(_buf)}};
  }

  _buf += ch;
  return {&Lexer::ReadWord};
}

Lexer::StateResult Lexer::ReadInteger(char ch, bool eof) {
  if (eof) {
    return {&Lexer::Idle, false, Lexeme{Type::VALUE_INT, std::stoi(_buf)}};
  }

  if (isdigit(ch)) {
    _buf += ch;
    return {&Lexer::ReadInteger};
  }

  if (ch == '.') {
    _buf += ch;
    return {&Lexer::ReadReal};
  }

  if (isliteral(ch)) {
    throw LexicalError{"Unexpected symbol"};
  }

  return {&Lexer::Idle, true, Lexeme{Type::VALUE_INT, std::stoi(_buf)}};
}

Lexer::StateResult Lexer::ReadReal(char ch, bool eof) {
  if (eof)
    return {&Lexer::Idle, false, Lexeme{Type::VALUE_INT, std::stod(_buf)}};

  if (isdigit(ch)) {
    _buf += ch;
    return {&Lexer::ReadReal};
  }

  return {&Lexer::Idle, true,
          Lexeme{Type::VALUE_REAL, std::stod(_buf)}};
}

Lexer::StateResult Lexer::ReadString(char ch, bool eof) {
  if (eof) throw LexicalError{"Unexpected end of file"};
  if (ch == '\n') throw LexicalError("Unexpected end of line");

  if (ch == '"') {
    return {&Lexer::Idle, false, Lexeme{Type::VALUE_STR, std::move(_buf)}};
  }

  _buf += ch;
  return {&Lexer::ReadString};
}

Lexer::StateResult Lexer::LineComment(char ch, bool eof) {
  if (ch == '\n') {
    return {&Lexer::Idle};
  }
  return {&Lexer::LineComment};
}

Lexer::StateResult Lexer::ManyLineCommentStar(char ch, bool eof) {
  if (ch == '*') {
    return {&Lexer::ManyLineCommentSlash};
  }
  return {&Lexer::ManyLineCommentStar};
}

Lexer::StateResult Lexer::ManyLineCommentSlash(char ch, bool eof) {
  if (ch == '/') {
    return {&Lexer::Idle};
  }
  return {&Lexer::ManyLineCommentSlash};
}

} // namespace interpreter::lex
