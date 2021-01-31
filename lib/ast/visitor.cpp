#include "interpreter/ast/visitor.hpp"

#include <iostream>
#include <unordered_map>

#include "interpreter/lexer/lexer.hpp"

namespace interpreter::ast {

namespace {

using LexType = lexer::LexType;

enum class ParseResult { SUCCESS, FAILURE };

// TODO: Is that ok?
[[nodiscard]] VariableType MapLexType2Type(LexType type) {
  if (type == LexType::TYPE_INT) return VariableType::INT;
  if (type == LexType::TYPE_REAL) return VariableType::REAL;
  if (type == LexType::TYPE_STR) return VariableType::STR;
  if (type == LexType::TYPE_BOOL) return VariableType::BOOL;

  throw SyntaxError{"Unexpected lexeme"};
}

[[nodiscard]] VariableType MapLexValue2Type(LexType type) {
  if (type == LexType::VALUE_INT) return VariableType::INT;
  if (type == LexType::VALUE_REAL) return VariableType::REAL;
  if (type == LexType::VALUE_STR) return VariableType::STR;
  if (type == LexType::FALSE) return VariableType::BOOL;
  if (type == LexType::TRUE) return VariableType::BOOL;

  throw SyntaxError{"Unexpected lexeme"};
}

inline const lexer::Lexeme& Validated(const lexer::Lexeme& lexeme,
                                      LexType required_type) {
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
  explicit constexpr ConstantParser(const LexType type) noexcept
      : type_{type} {}

  [[nodiscard]] Constant operator()(const std::monostate&) const {
    if (!lexer::IsBoolean(type_)) [[unlikely]] {
        throw SyntaxError{"Value lexeme should have value"};
      }

    return {VariableType::BOOL, type_ == LexType::TRUE};
  }

  template <typename T>
  [[nodiscard]] Constant operator()(const T& value) const {
    return {MapLexValue2Type(type_), value};
  }

 private:
  const LexType type_;
};

class ModelReader {
 public:
  explicit ModelReader(std::istream& code, ModelVisitor& visitor)
      : lex_generator_{lexer::ParseLexems(code)}, visitor_{visitor} {
    current_lex_it_ = lex_generator_.begin();
  }

  void VisitProgram() {
    ValidatedCurrentLex(LexType::PROGRAM);
    ValidatedMoveNextLex(LexType::OPENING_BRACE);
    visitor_.VisitProgram();

    MoveNextLex();
    VisitDeclarations();
    VisitOperators();

    ValidatedCurrentLex(LexType::CLOSING_BRACE);
  }

  Constant GetConstant() {
    const auto& constant = ValidatedCurrentLex(lexer::IsConstant);
    if (constant.type == LexType::FALSE || constant.type == LexType::TRUE) {
      return {VariableType::BOOL, constant.type == LexType::TRUE};
    }

    const auto current_lex = CurrentLex();
    MoveNextLex();

    return std::visit(ConstantParser(current_lex.type), current_lex.data);
  }

  void VisitVariableDeclaration(VariableType variable_type) {
    const auto& variable_name_lex = ValidatedCurrentLex(LexType::ID);
    auto variable_name = std::get<std::string>(variable_name_lex.data);

    std::optional<Constant> default_value;
    if (MoveNextLex().type == LexType::ASSIGN) {
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
    } while (CurrentLex().type == LexType::COMMA);

    return ParseResult::SUCCESS;
  }

  void VisitDeclarations() {
    visitor_.VisitDeclarations();

    while (VisitDescription() == ParseResult::SUCCESS) {
      ValidatedCurrentLex(LexType::SEMICOLON);
      MoveNextLex();
    }
  }

  ParseResult VisitRead() {
    if (CurrentLex().type != LexType::READ) {
      return ParseResult::FAILURE;
    }

    ValidatedMoveNextLex(LexType::OPENING_PARENTHESIS);

    auto variable_name =
        std::get<std::string>(ValidatedMoveNextLex(LexType::ID).data);
    visitor_.VisitRead(std::move(variable_name));

    ValidatedMoveNextLex(LexType::CLOSING_PARENTHESIS);
    ValidatedMoveNextLex(LexType::SEMICOLON);

    MoveNextLex();
    return ParseResult::SUCCESS;
  }

  ParseResult VisitWrite() {
    if (CurrentLex().type != LexType::WRITE) {
      return ParseResult::FAILURE;
    }

    ValidatedMoveNextLex(LexType::OPENING_PARENTHESIS);
    // TODO: list of expressions here please
    auto variable_name =
        std::get<std::string>(ValidatedMoveNextLex(LexType::ID).data);
    visitor_.VisitWrite(std::move(variable_name));

    ValidatedMoveNextLex(LexType::CLOSING_PARENTHESIS);
    ValidatedMoveNextLex(LexType::SEMICOLON);

    MoveNextLex();
    return ParseResult::SUCCESS;
  }

  ParseResult VisitCompoundOperator() {
    if (CurrentLex().type != LexType::OPENING_BRACE) {
      return ParseResult::FAILURE;
    }

    MoveNextLex();
    VisitOperators();
    ValidatedCurrentLex(LexType::CLOSING_BRACE);
    MoveNextLex();

    return ParseResult::SUCCESS;
  }

  ParseResult VisitAtom() {
    if (CurrentLex().type == LexType::OPENING_PARENTHESIS) {
      MoveNextLex();
      if (VisitExpression() == ParseResult::FAILURE) {
        throw ParseExpressionError{"Expression parse error"};
      }
      ValidatedCurrentLex(LexType::CLOSING_PARENTHESIS);
      MoveNextLex();

      return ParseResult::SUCCESS;
    }

    if (CurrentLex().type == LexType::ID) {
      std::string variable_name = std::get<std::string>(CurrentLex().data);
      visitor_.VisitVariableInvokation(std::move(variable_name));
      return ParseResult::SUCCESS;
    }

    if (lexer::IsConstant(CurrentLex().type)) {
      visitor_.VisitConstantInvokation(GetConstant());
      return ParseResult::SUCCESS;
    }

    return ParseResult::SUCCESS;
  }

  ParseResult VisitNot() {
    while (CurrentLex().type == LexType::NOT) {
      visitor_.VisitNot();
      MoveNextLex();
    }

    return VisitAtom();
  }

  ParseResult VisitMul() {
    if (VisitNot() == ParseResult::FAILURE) {
      return ParseResult::FAILURE;
    }

    while (CurrentLex().type == LexType::MUL ||
           CurrentLex().type == LexType::DIV ||
           CurrentLex().type == LexType::MOD) {
      MoveNextLex();
      if (VisitNot() == ParseResult::FAILURE) {
        throw ParseExpressionError{"Expression parse error"};
      }
      visitor_.VisitMul();
    }

    return ParseResult::SUCCESS;
  }

  ParseResult VisitAdd() {
    if (VisitMul() == ParseResult::FAILURE) {
      return ParseResult::FAILURE;
    }

    while (CurrentLex().type == LexType::PLUS ||
           CurrentLex().type == LexType::MINUS) {
      MoveNextLex();
      if (VisitMul() == ParseResult::FAILURE) {
        throw ParseExpressionError{"Expression parse error"};
      }
      visitor_.VisitAdd();
    }

    return ParseResult::SUCCESS;
  }

  ParseResult VisitCompare() {
    if (VisitAdd() == ParseResult::FAILURE) {
      return ParseResult::FAILURE;
    }

    while (lexer::IsCompare(CurrentLex().type)) {
      MoveNextLex();
      if (VisitAdd() == ParseResult::FAILURE) {
        throw ParseExpressionError{"Expression parse error"};
      }
      visitor_.VisitCompare();
    }

    return ParseResult::SUCCESS;
  }

  ParseResult VisitAnd() {
    if (VisitCompare() == ParseResult::FAILURE) {
      return ParseResult::FAILURE;
    }

    while (CurrentLex().type == LexType::AND) {
      MoveNextLex();
      if (VisitCompare() == ParseResult::FAILURE) {
        throw ParseExpressionError{"Expression parse error"};
      }
      visitor_.VisitAnd();
    }

    return ParseResult::SUCCESS;
  }

  ParseResult VisitOr() {
    if (VisitAnd() == ParseResult::FAILURE) {
      return ParseResult::FAILURE;
    }

    while (CurrentLex().type == LexType::OR) {
      MoveNextLex();
      if (VisitAnd() == ParseResult::FAILURE) {
        throw ParseExpressionError{"Expression parse error"};
      }
      visitor_.VisitOr();
    }

    return ParseResult::SUCCESS;
  }

  ParseResult VisitAssign() {
    if (VisitOr() == ParseResult::FAILURE) {
      return ParseResult::FAILURE;
    }

    if (CurrentLex().type == LexType::ASSIGN) {
      MoveNextLex();
      if (VisitOr() == ParseResult::FAILURE) {
        throw ParseExpressionError{"Expression parse error"};
      }
      visitor_.VisitAssign();
    }

    return ParseResult::SUCCESS;
  }

  ParseResult VisitExpression() { return VisitAssign(); }

  ParseResult VisitExpressionOperator() {
    if (VisitExpression() == ParseResult::FAILURE) {
      return ParseResult::FAILURE;
    }

    ValidatedCurrentLex(LexType::SEMICOLON);
    MoveNextLex();
    return ParseResult::SUCCESS;
  }

  ParseResult VisitOperator() {
    // using lazy evaluation here
    if (VisitRead() == ParseResult::SUCCESS ||
        VisitWrite() == ParseResult::SUCCESS ||
        VisitCompoundOperator() == ParseResult::SUCCESS ||
        VisitExpression() == ParseResult::SUCCESS) {
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
  inline const lexer::Lexeme& CurrentLex() const { return *current_lex_it_; }

  inline const lexer::Lexeme& ValidatedCurrentLex(LexType required_type) const {
    return Validated(*current_lex_it_, required_type);
  }

  template <typename TPredicate>
  inline const lexer::Lexeme& ValidatedCurrentLex(
      TPredicate&& predicate) const {
    return Validated(*current_lex_it_, std::forward<TPredicate>(predicate));
  }

  inline const lexer::Lexeme& MoveNextLex() { return *(++current_lex_it_); }

  inline const lexer::Lexeme& ValidatedMoveNextLex(LexType required_type) {
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
