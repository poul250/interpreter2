#include "interpreter/ast/visitor.hpp"

#include <iostream>
#include <unordered_map>

#include "interpreter/lexer/lexer.hpp"

namespace interpreter::ast {

namespace {

enum class ParseResult { SUCCESS, FAILURE };

// TODO: Is
[[nodiscard]] VariableType MapLexType2Type(lexer::LexType type) {
  using LexType = lexer::LexType;

  if (type == LexType::TYPE_INT) return VariableType::INT;
  if (type == LexType::TYPE_REAL) return VariableType::REAL;
  if (type == LexType::TYPE_STR) return VariableType::STR;
  if (type == LexType::TYPE_BOOL) return VariableType::BOOL;

  throw SyntaxError{"Unexpected lexeme"};
}

[[nodiscard]] VariableType MapLexValue2Type(lexer::LexType type) {
  using LexType = ::interpreter::lexer::LexType;

  if (type == LexType::VALUE_INT) return VariableType::INT;
  if (type == LexType::VALUE_REAL) return VariableType::REAL;
  if (type == LexType::VALUE_STR) return VariableType::STR;
  if (type == LexType::FALSE) return VariableType::BOOL;
  if (type == LexType::TRUE) return VariableType::BOOL;

  throw SyntaxError{"Unexpected lexeme"};
}

inline const lexer::Lexeme& Validated(const lexer::Lexeme& lexeme,
                                       lexer::LexType required_type) {
  // TODO: looks like clang-format bug, fix this
  if (lexeme.type != required_type) [[unlikely]] {
      throw SyntaxError{"Unexpected Lexeme"};
    }
  return lexeme;
}

template <typename TPredicate>
inline const lexer::Lexeme& Validated(const lexer::Lexeme& lexeme,
                                       TPredicate&& predicate) {
  if (!predicate(lexeme.type)) [[unlikely]] {
      throw SyntaxError{"Unexpected Lexeme"};
    }
  return lexeme;
}

class ConstantParser {
 public:
  explicit constexpr ConstantParser(const lexer::LexType type) noexcept
      : type_{type} {}

  [[nodiscard]] Constant operator()(const std::monostate&) const {
    if (!lexer::IsBoolean(type_)) [[unlikely]] {
        throw SyntaxError{"Value lexeme should have value"};
      }
    else {
      return {VariableType::BOOL, type_ == lexer::LexType::TRUE};
    }
  }

  template <typename T>
  [[nodiscard]] Constant operator()(const T& value) const {
    return {MapLexValue2Type(type_), value};
  }

 private:
  const lexer::LexType type_;
};

class ModelReader {
 public:
  explicit ModelReader(std::istream& code, ModelVisitor& visitor)
      : lex_generator_{lexer::ParseLexems(code)}, visitor_{visitor} {
    current_lex_it_ = lex_generator_.begin();
  }

  void VisitProgram() {
    ValidatedCurrentLex(lexer::LexType::PROGRAM);
    ValidatedMoveNextLex(lexer::LexType::OPENING_BRACE);
    visitor_.VisitProgram();

    MoveNextLex();
    VisitDescriptions();
    VisitOperators();

    ValidatedCurrentLex(lexer::LexType::CLOSING_BRACE);
  }

  Constant GetConstant() {
    const auto& constant = ValidatedCurrentLex(lexer::IsConstant);
    if (constant.type == lexer::LexType::FALSE ||
        constant.type == lexer::LexType::TRUE) {
      return {VariableType::BOOL, constant.type == lexer::LexType::TRUE};
    }

    const auto current_lex = CurrentLex();
    MoveNextLex();

    return std::visit(ConstantParser(current_lex.type), current_lex.data);
  }

  void VisitVariableDeclaration(VariableType variable_type) {
    const auto& variable_name_lex = ValidatedCurrentLex(lexer::LexType::ID);
    auto variable_name = std::get<std::string>(variable_name_lex.data);

    std::optional<Constant> default_value;
    if (MoveNextLex().type == lexer::LexType::ASSIGN) {
      MoveNextLex();
      default_value.emplace(GetConstant());
    }

    visitor_.VisitVariableDeclaration(variable_type, std::move(variable_name),
                                      std::move(default_value));
  }

  ParseResult VisitDescription() {
    const auto lex_type = CurrentLex().type;
    if (!lexer::IsVariableType(lex_type)) [[unlikely]] {
        return ParseResult::FAILURE;
      }
    const auto variable_type = MapLexType2Type(lex_type);

    do {
      MoveNextLex();
      VisitVariableDeclaration(variable_type);
    } while (CurrentLex().type == lexer::LexType::COMMA);

    return ParseResult::SUCCESS;
  }

  void VisitDescriptions() {
    visitor_.VisitDescriptions();

    while (VisitDescription() == ParseResult::SUCCESS) {
      ValidatedCurrentLex(lexer::LexType::SEMICOLON);
      MoveNextLex();
    }
  }

  ParseResult VisitRead() {
    if (CurrentLex().type != lexer::LexType::READ) {
      return ParseResult::FAILURE;
    }

    ValidatedMoveNextLex(lexer::LexType::OPENING_PARENTHESIS);

    auto variable_name =
        std::get<std::string>(ValidatedMoveNextLex(lexer::LexType::ID).data);
    visitor_.VisitRead(std::move(variable_name));

    ValidatedMoveNextLex(lexer::LexType::CLOSING_PARENTHESIS);
    ValidatedMoveNextLex(lexer::LexType::SEMICOLON);

    MoveNextLex();
    return ParseResult::SUCCESS;
  }

  ParseResult VisitWrite() {
    if (CurrentLex().type != lexer::LexType::WRITE) {
      return ParseResult::FAILURE;
    }

    ValidatedMoveNextLex(lexer::LexType::OPENING_PARENTHESIS);
    // TODO: list of expressions here please
    auto variable_name =
        std::get<std::string>(ValidatedMoveNextLex(lexer::LexType::ID).data);
    visitor_.VisitWrite(std::move(variable_name));

    ValidatedMoveNextLex(lexer::LexType::CLOSING_PARENTHESIS);
    ValidatedMoveNextLex(lexer::LexType::SEMICOLON);

    MoveNextLex();
    return ParseResult::SUCCESS;
  }

  ParseResult VisitOperator() {
    if (VisitRead() == ParseResult::SUCCESS) {
      return ParseResult::SUCCESS;
    }
    if (VisitWrite() == ParseResult::SUCCESS) {
      return ParseResult::SUCCESS;
    }

    return ParseResult::FAILURE;
  }

  void VisitOperators() {
    visitor_.VisitOperators();

    while (VisitOperator() == ParseResult::SUCCESS) {
      // Just visit while it lets us visit
    }
  }

 private:
  inline const lexer::Lexeme& CurrentLex() { return *current_lex_it_; }

  inline const lexer::Lexeme& ValidatedCurrentLex(lexer::LexType required_type) {
    return Validated(*current_lex_it_, required_type);
  }

  template <typename TPredicate>
  inline const lexer::Lexeme& ValidatedCurrentLex(TPredicate&& predicate) {
    return Validated(*current_lex_it_, std::forward<TPredicate>(predicate));
  }

  inline const lexer::Lexeme& MoveNextLex() { return *(++current_lex_it_); }

  inline const lexer::Lexeme& ValidatedMoveNextLex(
      lexer::LexType required_type) {
    return Validated(*(++current_lex_it_), required_type);
  }

  template <typename TPredicate>
  inline const lexer::Lexeme& ValidatedMoveNextLex(TPredicate&& predicate) {
    return Validated(*(++current_lex_it_), std::forward<TPredicate>(predicate));
  }

  utils::generator<lexer::Lexeme> lex_generator_;
  utils::generator<lexer::Lexeme>::iterator current_lex_it_;
  ModelVisitor& visitor_;
};

}  // namespace

void VisitCode(std::istream& code, ModelVisitor& visitor) {
  ModelReader{code, visitor}.VisitProgram();
}

}  // namespace interpreter::ast
