#include "interpreter/lex/lexems.hpp"

#include <functional>
#include <iostream>
#include <optional>
#include <unordered_map>
#include <unordered_set>

#include "interpreter/utils/format.hpp"

namespace interpreter::lexems {

namespace {

const std::unordered_map<std::string, Type> STRING_TO_TYPE = {
    {"and", Type::AND},
    {"boolean", Type::TYPE_BOOL},
    {"break", Type::BREAK},
    {"case", Type::CASE},
    {"continue", Type::CONTINUE},
    {"do", Type::DO},
    {"else", Type::ELSE},
    {"end", Type::END},
    {"false", Type::FALSE},
    {"for", Type::FOR},
    {"if", Type::IF},
    {"int", Type::TYPE_INT},
    {"not", Type::NOT},
    {"of", Type::OF},
    {"or", Type::OR},
    {"program", Type::PROGRAM},
    {"read", Type::READ},
    {"real", Type::TYPE_REAL},
    {"string", Type::TYPE_STR},
    {"true", Type::TRUE},
    {"while", Type::WHILE},
    {"write", Type::WRITE},

    {"!=", Type::NE},
    {"==", Type::EQ},
    {"<=", Type::LE},
    {">=", Type::GE},
};

const std::unordered_set<char> COMPLEX_CHARS = {'!', '=', '<', '>', '/'};

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

[[nodiscard]] constexpr bool IsLiteral(char ch) noexcept {
  return isdigit(ch) || isalpha(ch) || ch == '_';
}

template <typename Key, typename Value>
[[nodiscard]] std::unordered_map<Value, Key> ReverseMap(
    const std::unordered_map<Key, Value>& map) {
  std::unordered_map<Value, Key> result;
  for (const auto& [key, value] : map) {
    result.emplace(value, key);
  }
  return result;
}

class Lexer {
 public:
  explicit Lexer(std::istream& input, int line = 1) noexcept;

  Lexeme GetNext();
  [[nodiscard]] inline int CurrentLine() const noexcept { return line_; }

 private:
  struct StateResult;
  using State = std::function<StateResult(Lexer&, char)>;

  struct StateResult {
    State next_state;
    bool need_unget = false;
    std::optional<Lexeme> parsed_lexeme = std::nullopt;
  };

  StateResult Idle(char ch);
  StateResult ComplexOp(char ch);
  StateResult ReadWord(char ch);
  StateResult ReadInteger(char ch);
  StateResult ReadReal(char ch);
  StateResult LineComment(char ch);
  StateResult ManyLineCommentStar(char ch);
  StateResult ManyLineCommentSlash(char ch);
  StateResult ReadString(char ch);

  int line_;
  char prev_ch_ = 0;
  bool eof_ = false;
  std::string buf_;
  State current_state_;
  std::istream& input_;
};

Lexer::Lexer(std::istream& input, int line) noexcept
    : input_(input), line_(line), current_state_{&Lexer::Idle} {}

Lexeme Lexer::GetNext() {
  if (input_.eof()) return {};

  buf_.clear();

  char ch;
  while (input_.get(ch)) {
    const auto& [next_state, need_unget, opt_lexeme] =
        current_state_(*this, ch);
    current_state_ = next_state;

    if (need_unget) {
      input_.unget();
    } else {
      line_ += ch == '\n';
    }

    prev_ch_ = ch;
    if (opt_lexeme) {
      return *opt_lexeme;
    }
  }

  eof_ = true;
  const auto& [_, __, opt_lexeme] = current_state_(*this, ch);
  return opt_lexeme.value_or(Lexeme{});
}

Lexer::StateResult Lexer::Idle(char ch) {
  if (eof_ || isspace(ch)) {
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

  if (COMPLEX_CHARS.contains(ch)) {
    return {&Lexer::ComplexOp};
  }

  if (auto it = SINGLE_CHAR.find(ch); it != SINGLE_CHAR.end()) {
    return {&Lexer::Idle, false, Lexeme{it->second}};
  }

  throw LexicalError{utils::format("Unrecognized symbol '{}'", ch)};
}

Lexer::StateResult Lexer::ComplexOp(char ch) {
  if (eof_) {
    if (auto it = SINGLE_CHAR.find(prev_ch_); it != SINGLE_CHAR.end()) {
      return {&Lexer::Idle, false, Lexeme{it->second}};
    }
    throw LexicalError{"Unexpected end of file"};
  }

  if (prev_ch_ == '/') {
    if (ch == '/') return {&Lexer::LineComment};
    if (ch == '*') return {&Lexer::ManyLineCommentStar};

    return {&Lexer::Idle, true, Lexeme{Type::DIV}};
  }

  std::string complex_op = std::string{prev_ch_} + ch;
  if (auto it = STRING_TO_TYPE.find(complex_op); it != STRING_TO_TYPE.end()) {
    return {&Lexer::Idle, false, Lexeme{it->second}};
  }

  if (auto it = SINGLE_CHAR.find(prev_ch_); it != SINGLE_CHAR.end()) {
    return {&Lexer::Idle, true, Lexeme{it->second}};
  }

  throw LexicalError{utils::format("Unexpected symbol '{}'", prev_ch_)};
}

Lexer::StateResult Lexer::ReadWord(char ch) {
  if (eof_ || !isalpha(ch)) {
    auto it = STRING_TO_TYPE.find(buf_);
    if (it != STRING_TO_TYPE.end()) {
      return {&Lexer::Idle, !eof_, Lexeme{it->second}};
    }
    return {&Lexer::Idle, !eof_, Lexeme{Type::ID, std::move(buf_)}};
  }

  buf_ += ch;
  return {&Lexer::ReadWord};
}

Lexer::StateResult Lexer::ReadInteger(char ch) {
  if (eof_) {
    return {&Lexer::Idle, false, Lexeme{Type::VALUE_INT, std::stoi(buf_)}};
  }

  if (isdigit(ch)) {
    buf_ += ch;
    return {&Lexer::ReadInteger};
  }

  if (ch == '.') {
    buf_ += ch;
    return {&Lexer::ReadReal};
  }

  if (IsLiteral(ch)) {
    throw LexicalError{"Unexpected symbol"};
  }

  return {&Lexer::Idle, true, Lexeme{Type::VALUE_INT, std::stoi(buf_)}};
}

Lexer::StateResult Lexer::ReadReal(char ch) {
  if (eof_)
    return {&Lexer::Idle, false, Lexeme{Type::VALUE_INT, std::stod(buf_)}};

  if (isdigit(ch)) {
    buf_ += ch;
    return {&Lexer::ReadReal};
  }

  return {&Lexer::Idle, true, Lexeme{Type::VALUE_REAL, std::stod(buf_)}};
}

Lexer::StateResult Lexer::ReadString(char ch) {
  if (eof_) throw LexicalError{"Unexpected end of file"};
  if (ch == '\n') throw LexicalError("Unexpected end of line");

  if (ch == '"') {
    return {&Lexer::Idle, false, Lexeme{Type::VALUE_STR, std::move(buf_)}};
  }

  buf_ += ch;
  return {&Lexer::ReadString};
}

Lexer::StateResult Lexer::LineComment(char ch) {
  if (ch == '\n') {
    return {&Lexer::Idle};
  }
  return {&Lexer::LineComment};
}

Lexer::StateResult Lexer::ManyLineCommentStar(char ch) {
  if (ch == '*') {
    return {&Lexer::ManyLineCommentSlash};
  }
  return {&Lexer::ManyLineCommentStar};
}

Lexer::StateResult Lexer::ManyLineCommentSlash(char ch) {
  if (ch == '/') {
    return {&Lexer::Idle};
  }
  return {&Lexer::ManyLineCommentSlash};
}

}  // namespace

utils::generator<Lexeme> ParseLexems(std::istream& input) {
  // TODO: looks weird, pls do something with this
  Lexer lexer(input);
  Lexeme lex;
  do {
    lex = lexer.GetNext();
    co_yield lex;
  } while (lex.type != Type::NONE);
}

}  // namespace interpreter::lexems
