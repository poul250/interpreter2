#include "interpreter/ast/visitor.hpp"

#include <iostream>
#include <ranges>
#include <unordered_map>

#include "interpreter/lexer/lexer.hpp"

namespace interpreter::ast {

namespace {

using LexType = lexer::LexType;

enum class ParseResult : bool { FAILURE = false, SUCCESS = true };

// TODO: Is that ok?
[[nodiscard]] inline VariableType MapType(LexType type) {
  switch (type) {
    case LexType::TYPE_INT:
      return VariableType::INT;
    case LexType::TYPE_REAL:
      return VariableType::REAL;
    case LexType::TYPE_STR:
      return VariableType::STR;
    case LexType::TYPE_BOOL:
      return VariableType::BOOL;
  }

  throw SyntaxError{"Unexpected lexeme"};
}

[[nodiscard]] inline VariableType MapValue(LexType type) {
  switch (type) {
    case LexType::VALUE_INT:
      return VariableType::INT;
    case LexType::VALUE_REAL:
      return VariableType::REAL;
    case LexType::VALUE_STR:
      return VariableType::STR;
    case LexType::FALSE:
      return VariableType::BOOL;
    case LexType::TRUE:
      return VariableType::BOOL;
  }

  throw SyntaxError{"Unexpected lexeme"};
}

[[nodiscard]] inline CompareType MapCompare(LexType type) {
  switch (type) {
    case LexType::LT:
      return CompareType::LT;
    case LexType::GT:
      return CompareType::GT;
    case LexType::LE:
      return CompareType::LE;
    case LexType::GE:
      return CompareType::GE;
    case LexType::EQ:
      return CompareType::EQ;
    case LexType::NE:
      return CompareType::NE;
  }

  throw SyntaxError{"Unexpected lexeme"};
}

[[nodiscard]] inline MulType MapMul(LexType type) {
  switch (type) {
    case LexType::MUL:
      return MulType::MUL;
    case LexType::DIV:
      return MulType::DIV;
    case LexType::MOD:
      return MulType::MOD;
  }

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
  if (!std::forward<TPredicate>(predicate)(lexeme.type)) [[unlikely]] {
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
    return {MapValue(type_), value};
  }

 private:
  const LexType type_;
};

template <typename Range>
class ModelReader {
 public:
  explicit ModelReader() = delete;
  explicit ModelReader(Range& range, ModelVisitor& visitor)
      : current_lex_it_{range.begin()}, visitor_{visitor} {}

  Constant GetConstant() {
    const auto constant = Validated(Current(), lexer::IsConstant);
    MoveNext();

    if (constant.type == LexType::FALSE || constant.type == LexType::TRUE) {
      return {VariableType::BOOL, constant.type == LexType::TRUE};
    }

    return std::visit(ConstantParser(constant.type), constant.data);
  }

  ParseResult VisitAtom() {
    if (Current().type == LexType::OPENING_PARENTHESIS) {
      MoveNext();
      if (VisitExpression() == ParseResult::FAILURE) {
        throw ParseExpressionError{"Expression parse error"};
      }
      Validated(Current(), LexType::CLOSING_PARENTHESIS);
      MoveNext();

      return ParseResult::SUCCESS;
    }

    if (Current().type == LexType::ID) {
      std::string variable_name = std::get<std::string>(Current().data);
      visitor_.VisitVariableInvokation(std::move(variable_name));
      MoveNext();
      return ParseResult::SUCCESS;
    }

    if (lexer::IsConstant(Current().type)) {
      visitor_.VisitConstantInvokation(GetConstant());
      return ParseResult::SUCCESS;
    }

    return ParseResult::FAILURE;
  }

  ParseResult VisitNot() {
    bool has_not = false;
    if (Current().type == LexType::NOT) {
      has_not = true;
      MoveNext();
    }

    const auto atom_result = VisitAtom();
    if (has_not && atom_result == ParseResult::FAILURE) {
      throw ParseExpressionError{"Missing expression after not"};
    }
    if (has_not) {
      visitor_.VisitNot();
    }
    return atom_result;
  }

  ParseResult VisitMul() {
    if (VisitNot() == ParseResult::FAILURE) {
      return ParseResult::FAILURE;
    }

    while (Current().type == LexType::MUL || Current().type == LexType::DIV ||
           Current().type == LexType::MOD) {
      const auto mul_type = MapMul(Current().type);
      MoveNext();
      if (VisitNot() == ParseResult::FAILURE) {
        throw ParseExpressionError{"Expression parse error"};
      }
      visitor_.VisitMul(mul_type);
    }

    return ParseResult::SUCCESS;
  }

  ParseResult VisitAdd() {
    if (VisitMul() == ParseResult::FAILURE) {
      return ParseResult::FAILURE;
    }

    while (Current().type == LexType::PLUS ||
           Current().type == LexType::MINUS) {
      const auto add_type =
          Current().type == LexType::PLUS ? AddType::PLUS : AddType::MINUS;

      MoveNext();
      if (VisitMul() == ParseResult::FAILURE) {
        throw ParseExpressionError{"Expression parse error"};
      }
      visitor_.VisitAdd(add_type);
    }

    return ParseResult::SUCCESS;
  }

  ParseResult VisitCompare() {
    if (VisitAdd() == ParseResult::FAILURE) {
      return ParseResult::FAILURE;
    }

    while (lexer::IsCompare(Current().type)) {
      const auto compare_type = MapCompare(Current().type);
      MoveNext();
      if (VisitAdd() == ParseResult::FAILURE) {
        throw ParseExpressionError{"Expression parse error"};
      }
      visitor_.VisitCompare(compare_type);
    }

    return ParseResult::SUCCESS;
  }

  ParseResult VisitAnd() {
    if (VisitCompare() == ParseResult::FAILURE) {
      return ParseResult::FAILURE;
    }

    while (Current().type == LexType::AND) {
      MoveNext();
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

    while (Current().type == LexType::OR) {
      MoveNext();
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

    if (Current().type == LexType::ASSIGN) {
      MoveNext();
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
    Validated(Current(), LexType::SEMICOLON);

    visitor_.VisitExpressionOperator();

    MoveNext();
    return ParseResult::SUCCESS;
  }

  ParseResult VisitCompoundOperator() {
    if (Current().type != LexType::OPENING_BRACE) {
      return ParseResult::FAILURE;
    }
    MoveNext();
    VisitOperators();
    Validated(Current(), LexType::CLOSING_BRACE);
    MoveNext();

    return ParseResult::SUCCESS;
  }

  ParseResult VisitWrite() {
    if (Current().type != LexType::WRITE) {
      return ParseResult::FAILURE;
    }

    Validated(MoveNext(), LexType::OPENING_PARENTHESIS);

    do {
      MoveNext();
      if (VisitExpression() == ParseResult::FAILURE) {
        throw ParseExpressionError{"Expression parse error"};
      }
      visitor_.VisitWrite();
    } while (Current().type == LexType::COMMA);

    Validated(Current(), LexType::CLOSING_PARENTHESIS);
    Validated(MoveNext(), LexType::SEMICOLON);
    MoveNext();
    return ParseResult::SUCCESS;
  }

  ParseResult VisitRead() {
    if (Current().type != LexType::READ) {
      return ParseResult::FAILURE;
    }

    Validated(MoveNext(), LexType::OPENING_PARENTHESIS);

    auto variable_name =
        std::get<std::string>(Validated(MoveNext(), LexType::ID).data);
    visitor_.VisitRead(std::move(variable_name));

    Validated(MoveNext(), LexType::CLOSING_PARENTHESIS);
    Validated(MoveNext(), LexType::SEMICOLON);

    MoveNext();
    return ParseResult::SUCCESS;
  }

  ParseResult VisitContinue() {
    if (Current().type != LexType::CONTINUE) {
      return ParseResult::FAILURE;
    }
    Validated(MoveNext(), LexType::SEMICOLON);
    visitor_.VisitContinue();
    MoveNext();
    return ParseResult::SUCCESS;
  }

  ParseResult VisitBreak() {
    if (Current().type != LexType::BREAK) {
      return ParseResult::FAILURE;
    }
    Validated(MoveNext(), LexType::SEMICOLON);
    visitor_.VisitBreak();
    MoveNext();
    return ParseResult::SUCCESS;
  }

  ParseResult VisitWhile() {
    if (Current().type != LexType::WHILE) {
      return ParseResult::FAILURE;
    }
    visitor_.VisitWhile();

    Validated(MoveNext(), LexType::OPENING_PARENTHESIS);
    MoveNext();
    if (VisitExpression() == ParseResult::FAILURE) {
      throw ParseOperatorError{"Failed to parse while expression"};
    }
    Validated(Current(), LexType::CLOSING_PARENTHESIS);

    visitor_.VisitWhileBody();
    MoveNext();
    if (VisitOperator() == ParseResult::FAILURE) {
      throw ParseOperatorError{"Failed to parse while body"};
    }
    visitor_.VisitEndWhile();

    return ParseResult::SUCCESS;
  }

  ParseResult VisitIf() {
    if (Current().type != LexType::IF) {
      return ParseResult::FAILURE;
    }

    Validated(MoveNext(), LexType::OPENING_PARENTHESIS);
    MoveNext();
    if (VisitExpression() == ParseResult::FAILURE) {
      throw ParseOperatorError{"Failed to parse if expression"};
    }
    Validated(Current(), LexType::CLOSING_PARENTHESIS);

    visitor_.VisitIf();
    MoveNext();
    if (VisitOperator() == ParseResult::FAILURE) {
      throw ParseOperatorError{"Failed to parse if(true) operation"};
    }

    if (Current().type == LexType::ELSE) {
      visitor_.VisitElse();
      MoveNext();
      if (VisitOperator() == ParseResult::FAILURE) {
        throw ParseOperatorError{"Failed to parse if(false) operation"};
      }
    }
    visitor_.VisitEndIf();

    return ParseResult::SUCCESS;
  }

  ParseResult VisitOperator() {
    // using lazy evaluation here
    if (VisitIf() == ParseResult::SUCCESS ||
        VisitWhile() == ParseResult::SUCCESS ||
        VisitBreak() == ParseResult::SUCCESS ||
        VisitContinue() == ParseResult::SUCCESS ||
        VisitRead() == ParseResult::SUCCESS ||
        VisitWrite() == ParseResult::SUCCESS ||
        VisitCompoundOperator() == ParseResult::SUCCESS ||
        VisitExpressionOperator() == ParseResult::SUCCESS) {
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

  void VisitVariableDeclaration(VariableType variable_type) {
    const auto& variable_name_lex = Validated(Current(), LexType::ID);
    auto variable_name = std::get<std::string>(variable_name_lex.data);

    std::optional<Constant> default_value;
    if (MoveNext().type == LexType::ASSIGN) {
      MoveNext();
      default_value.emplace(GetConstant());
    }

    visitor_.VisitVariableDeclaration(variable_type, std::move(variable_name),
                                      std::move(default_value));
  }

  ParseResult VisitDeclaration() {
    const auto lex_type = Current().type;
    if (!lexer::IsVariableType(lex_type)) [[unlikely]] {
        return ParseResult::FAILURE;
      }
    const auto variable_type = MapType(lex_type);

    do {
      MoveNext();
      VisitVariableDeclaration(variable_type);
    } while (Current().type == LexType::COMMA);

    return ParseResult::SUCCESS;
  }

  void VisitDeclarations() {
    visitor_.VisitDeclarations();

    while (VisitDeclaration() == ParseResult::SUCCESS) {
      Validated(Current(), LexType::SEMICOLON);
      MoveNext();
    }
  }

  void VisitProgram() {
    Validated(Current(), LexType::PROGRAM);
    Validated(MoveNext(), LexType::OPENING_BRACE);
    visitor_.VisitProgram();

    MoveNext();
    VisitDeclarations();
    VisitOperators();

    Validated(Current(), LexType::CLOSING_BRACE);
  }

 private:
  // TODO: add end() checks
  inline const lexer::Lexeme& Current() const { return *current_lex_it_; }

  inline const lexer::Lexeme& MoveNext() { return *(++current_lex_it_); }

  typename Range::iterator current_lex_it_;
  ModelVisitor& visitor_;
};

}  // namespace

// sorry about non-const references
void VisitCode(std::istream& code, ModelVisitor& visitor) {
  auto lexems_generator = lexer::ParseLexems(code);
  ModelReader(lexems_generator, visitor).VisitProgram();
}

}  // namespace interpreter::ast
