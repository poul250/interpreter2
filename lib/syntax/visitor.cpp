#include "interpreter/syntax/visitor.hpp"

#include <iostream>
#include <unordered_map>

#include "interpreter/lexer/lexems.hpp"

namespace interpreter::syntax {

namespace {

enum class ParseResult { SUCCESS, FAILURE };

// TODO: Is it good mapping?
[[nodiscard]] VariableType MapLexType2Type(lexems::Type type) {
  using LexType = ::interpreter::lexems::Type;

  if (type == LexType::TYPE_INT) return VariableType::INT;
  if (type == LexType::TYPE_REAL) return VariableType::REAL;
  if (type == LexType::TYPE_STR) return VariableType::STR;
  if (type == LexType::TYPE_BOOL) return VariableType::BOOL;

  throw SyntaxError{"Unexpected lexeme"};
}

[[nodiscard]] VariableType MapLexValue2Type(lexems::Type type) {
  using LexType = ::interpreter::lexems::Type;

  if (type == LexType::VALUE_INT) return VariableType::INT;
  if (type == LexType::VALUE_REAL) return VariableType::REAL;
  if (type == LexType::VALUE_STR) return VariableType::STR;
  if (type == LexType::FALSE) return VariableType::BOOL;
  if (type == LexType::TRUE) return VariableType::BOOL;

  throw SyntaxError{"Unexpected lexeme"};
}

inline const lexems::Lexeme& Validated(const lexems::Lexeme& lexeme,
                                       lexems::Type required_type) {
  // TODO: looks like clang-format bug, fix this
  if (lexeme.type != required_type) [[unlikely]] {
      throw SyntaxError{"Unexpected Lexeme"};
    }
  return lexeme;
}

template <typename TPredicate>
inline const lexems::Lexeme& Validated(const lexems::Lexeme& lexeme,
                                       TPredicate&& predicate) {
  if (!predicate(lexeme.type)) [[unlikely]] {
      throw SyntaxError{"Unexpected Lexeme"};
    }
  return lexeme;
}

class ConstantParser {
 public:
  explicit constexpr ConstantParser(const lexems::Type type) noexcept
      : type_{type} {}

  [[nodiscard]] Constant operator()(const std::monostate&) const {
    if (!lexems::IsBoolean(type_)) [[unlikely]] {
        throw SyntaxError{"Value lexeme should have value"};
      }
    else {
      return {VariableType::BOOL, type_ == lexems::Type::TRUE};
    }
  }

  template <typename T>
  [[nodiscard]] Constant operator()(const T& value) const {
    return {MapLexValue2Type(type_), value};
  }

 private:
  const lexems::Type type_;
};

class ModelReader {
 public:
  explicit ModelReader(std::istream& code, ModelVisitor& visitor)
      : lex_generator_{lexems::ParseLexems(code)}, visitor_{visitor} {
    current_lex_it_ = lex_generator_.begin();
  }

  void VisitProgram() {
    ValidatedCurrentLex(lexems::Type::PROGRAM);
    ValidatedMoveNextLex(lexems::Type::OPENING_BRACE);
    visitor_.VisitProgram();

    MoveNextLex();
    VisitDescriptions();
    VisitOperators();

    ValidatedCurrentLex(lexems::Type::CLOSING_BRACE);
  }

  Constant GetConstant() {
    const auto& constant = ValidatedCurrentLex(lexems::IsConstant);
    if (constant.type == lexems::Type::FALSE ||
        constant.type == lexems::Type::TRUE) {
      return {VariableType::BOOL, constant.type == lexems::Type::TRUE};
    }

    const auto current_lex = CurrentLex();
    MoveNextLex();

    return std::visit(ConstantParser(current_lex.type), current_lex.data);
  }

  void VisitVariableDeclaration(VariableType variable_type) {
    const auto& variable_name_lex = ValidatedCurrentLex(lexems::Type::ID);
    auto variable_name = std::get<std::string>(variable_name_lex.data);

    std::optional<Constant> default_value;
    if (MoveNextLex().type == lexems::Type::ASSIGN) {
      MoveNextLex();
      default_value.emplace(GetConstant());
    }

    visitor_.VisitVariableDeclaration(variable_type, std::move(variable_name),
                                      std::move(default_value));
  }

  ParseResult VisitDescription() {
    const auto lex_type = CurrentLex().type;
    if (!lexems::IsVariableType(lex_type)) [[unlikely]] {
        return ParseResult::FAILURE;
      }
    const auto variable_type = MapLexType2Type(lex_type);

    do {
      MoveNextLex();
      VisitVariableDeclaration(variable_type);
    } while (CurrentLex().type == lexems::Type::COMMA);

    return ParseResult::SUCCESS;
  }

  void VisitDescriptions() {
    visitor_.VisitDescriptions();

    while (VisitDescription() == ParseResult::SUCCESS) {
      ValidatedCurrentLex(lexems::Type::SEMICOLON);
      MoveNextLex();
    }
  }

  ParseResult VisitRead() {
    if (CurrentLex().type != lexems::Type::READ) {
      return ParseResult::FAILURE;
    }

    ValidatedMoveNextLex(lexems::Type::OPENING_PARENTHESIS);

    auto variable_name =
        std::get<std::string>(ValidatedMoveNextLex(lexems::Type::ID).data);
    visitor_.VisitRead(std::move(variable_name));

    ValidatedMoveNextLex(lexems::Type::CLOSING_PARENTHESIS);
    ValidatedMoveNextLex(lexems::Type::SEMICOLON);

    MoveNextLex();
    return ParseResult::SUCCESS;
  }

  ParseResult VisitWrite() {
    if (CurrentLex().type != lexems::Type::WRITE) {
      return ParseResult::FAILURE;
    }

    ValidatedMoveNextLex(lexems::Type::OPENING_PARENTHESIS);
    // TODO: list of expressions here please
    auto variable_name =
        std::get<std::string>(ValidatedMoveNextLex(lexems::Type::ID).data);
    visitor_.VisitWrite(std::move(variable_name));

    ValidatedMoveNextLex(lexems::Type::CLOSING_PARENTHESIS);
    ValidatedMoveNextLex(lexems::Type::SEMICOLON);

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
  inline const lexems::Lexeme& CurrentLex() { return *current_lex_it_; }

  inline const lexems::Lexeme& ValidatedCurrentLex(lexems::Type required_type) {
    return Validated(*current_lex_it_, required_type);
  }

  template <typename TPredicate>
  inline const lexems::Lexeme& ValidatedCurrentLex(TPredicate&& predicate) {
    return Validated(*current_lex_it_, std::forward<TPredicate>(predicate));
  }

  inline const lexems::Lexeme& MoveNextLex() { return *(++current_lex_it_); }

  inline const lexems::Lexeme& ValidatedMoveNextLex(
      lexems::Type required_type) {
    return Validated(*(++current_lex_it_), required_type);
  }

  template <typename TPredicate>
  inline const lexems::Lexeme& ValidatedMoveNextLex(TPredicate&& predicate) {
    return Validated(*(++current_lex_it_), std::forward<TPredicate>(predicate));
  }

  utils::generator<lexems::Lexeme> lex_generator_;
  utils::generator<lexems::Lexeme>::iterator current_lex_it_;
  ModelVisitor& visitor_;
};

}  // namespace

void VisitCode(std::istream& code, ModelVisitor& visitor) {
  ModelReader{code, visitor}.VisitProgram();
}

}  // namespace interpreter::syntax
