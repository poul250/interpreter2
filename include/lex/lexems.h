#pragma once

#include <iosfwd>
#include <optional>
#include <stdexcept>
#include <string>
#include <variant>

#include "types.h"

namespace interpreter::lexems {

class LexicalError : public std::runtime_error {
public:
  inline explicit LexicalError(const std::string& error) : std::runtime_error{error} {
  }
};

struct Lexeme {
  Lexeme(const Lexeme&) = default;
  Lexeme(Lexeme&&) = default;

  Lexeme& operator=(const Lexeme&) = default;
  Lexeme& operator=(Lexeme&&) = default;

  Type type;
  std::variant<std::monostate, int, double, bool, std::string> data;
};

class Lexer {
public:
  explicit Lexer(std::istream& input, int line = 1);

  Lexeme GetNext();
  [[nodiscard]] inline int CurrentLine() const noexcept { return _line; }
private:
  struct StateResult;
  using State = std::function<StateResult(Lexer&, char, bool)>;

  struct StateResult {
    StateResult(State next_state, bool need_unget = false,
                std::optional<Lexeme> parsed_lexeme = std::nullopt)
            : next_state(std::move(next_state)), need_unget(need_unget),
              parsed_lexeme(std::move(parsed_lexeme)) { }

    State next_state;
    bool need_unget;
    std::optional<Lexeme> parsed_lexeme;
  };

  StateResult Idle(char ch, bool eof);
  StateResult ComplexOp(char ch, bool eof);
  StateResult ReadWord(char ch, bool eof);
  StateResult ReadInteger(char ch, bool eof);
  StateResult ReadReal(char ch, bool eof);
  StateResult LineComment(char ch, bool eof);
  StateResult ManyLineCommentStar(char ch, bool eof);
  StateResult ManyLineCommentSlash(char ch, bool eof);
  StateResult ReadString(char ch, bool eof);

  int _line;
  char _prev_ch = 0;
  std::string _buf;
  State _current_state;

  std::istream& _input;
};

std::ostream& operator<<(std::ostream& out, const Lexeme& lexeme);

} // namespace interpreter::lexems
