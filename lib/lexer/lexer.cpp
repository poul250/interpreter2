#include "interpreter/lexer/lexer.hpp"

#include <iostream>
#include <optional>
#include <unordered_map>
#include <unordered_set>

#include "interpreter/utils/format.hpp"

namespace interpreter::lexer {

namespace {

const std::unordered_map<std::string, LexType> STRING_TO_TYPE = {
    {"and", LexType::AND},
    {"boolean", LexType::TYPE_BOOL},
    {"break", LexType::BREAK},
    {"case", LexType::CASE},
    {"continue", LexType::CONTINUE},
    {"do", LexType::DO},
    {"else", LexType::ELSE},
    {"end", LexType::END},
    {"false", LexType::FALSE},
    {"for", LexType::FOR},
    {"if", LexType::IF},
    {"int", LexType::TYPE_INT},
    {"not", LexType::NOT},
    {"of", LexType::OF},
    {"or", LexType::OR},
    {"program", LexType::PROGRAM},
    {"read", LexType::READ},
    {"real", LexType::TYPE_REAL},
    {"string", LexType::TYPE_STR},
    {"true", LexType::TRUE},
    {"while", LexType::WHILE},
    {"write", LexType::WRITE},

    {"!=", LexType::NE},
    {"==", LexType::EQ},
    {"<=", LexType::LE},
    {">=", LexType::GE},
};

const std::unordered_set<char> COMPLEX_CHARS = {'!', '=', '<', '>', '/'};

const std::unordered_map<char, LexType> SINGLE_CHAR = {
    {'=', LexType::ASSIGN},
    {'<', LexType::LT},
    {'>', LexType::GT},
    {'/', LexType::DIV},
    {'+', LexType::PLUS},
    {'-', LexType::MINUS},
    {'%', LexType::MOD},
    {'*', LexType::MUL},
    {';', LexType::SEMICOLON},
    {',', LexType::COMMA},
    {'{', LexType::OPENING_BRACE},
    {'}', LexType::CLOSING_BRACE},
    {'(', LexType::OPENING_PARENTHESIS},
    {')', LexType::CLOSING_PARENTHESIS},
};

const std::unordered_map<char, char> ESCAPE_CHARACTERS = {
    {'n', '\n'}, {'t', '\t'}, {'r', '\r'}, {'"', '\"'}, {'\\', '\\'}};

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
  StateResult ReadEscapeCharacter(char ch);

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

    return {&Lexer::Idle, true, Lexeme{LexType::DIV}};
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
    return {&Lexer::Idle, !eof_, Lexeme{LexType::ID, std::move(buf_)}};
  }

  buf_ += ch;
  return {&Lexer::ReadWord};
}

Lexer::StateResult Lexer::ReadInteger(char ch) {
  if (eof_) {
    return {&Lexer::Idle, false, Lexeme{LexType::VALUE_INT, std::stoi(buf_)}};
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

  return {&Lexer::Idle, true, Lexeme{LexType::VALUE_INT, std::stoi(buf_)}};
}

Lexer::StateResult Lexer::ReadReal(char ch) {
  if (eof_)
    return {&Lexer::Idle, false, Lexeme{LexType::VALUE_INT, std::stod(buf_)}};

  if (isdigit(ch)) {
    buf_ += ch;
    return {&Lexer::ReadReal};
  }

  return {&Lexer::Idle, true, Lexeme{LexType::VALUE_REAL, std::stod(buf_)}};
}

Lexer::StateResult Lexer::ReadString(char ch) {
  if (eof_) throw LexicalError{"Unexpected end of file"};
  if (ch == '\n') throw LexicalError("Unexpected end of line");

  if (ch == '\\') {
    return {&Lexer::ReadEscapeCharacter};
  }
  if (ch == '"') {
    return {&Lexer::Idle, false, Lexeme{LexType::VALUE_STR, std::move(buf_)}};
  }

  buf_ += ch;
  return {&Lexer::ReadString};
}

Lexer::StateResult Lexer::ReadEscapeCharacter(char ch) {
  if (eof_) throw LexicalError{"Unexpected end of file"};

  if (auto it = ESCAPE_CHARACTERS.find(ch); it != ESCAPE_CHARACTERS.cend()) {
    buf_ += it->second;
  } else {
    buf_ += ch;
  }

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
  } while (lex.type != LexType::NONE);
}

}  // namespace interpreter::lexer
