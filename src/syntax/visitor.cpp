#include <iostream>
#include <unordered_map>

#include <syntax/visitor.h>

namespace interpreter::syntax {

namespace {
namespace tu = lexems::type_utils;

Type GetTypeByLexType(lexems::Type type) {
  if (type == lexems::Type::TYPE_INT) return Type::INT;
  else if (type == lexems::Type::TYPE_REAL) return Type::REAL;
  else if (type == lexems::Type::TYPE_STR) return Type::STR;
  else if (type == lexems::Type::TYPE_BOOL) return Type::BOOL;

  throw SyntaxError{"Unexpected lexeme"};
}

class ModelReader {
public:
  ModelReader(std::istream& code, ModelVisitor& visitor) : _visitor{visitor} {
    lexems::Lexer lexer{code};
    lexems::Type last_type;

    // TODO: looks bad, do something with this
    do {
      auto&& lexeme = lexer.GetNext();
      last_type = lexeme.type;
      _lexems.emplace_back(std::move(lexeme));
    } while (last_type != lexems::Type::NONE);
  }

  void VisitProgram() {
    Current(lexems::Type::PROGRAM);
    GetNext(lexems::Type::OPENING_BRACE);
    _visitor.VisitProgram();

    GetNext();
    VisitDescriptions();
    VisitOperators();

    Current(lexems::Type::CLOSING_BRACE);
  }

  Constant GetConstant() {
    const auto& constant = GetNext(tu::IsConstant);
    if (constant.type == lexems::Type::FALSE || constant.type == lexems::Type::TRUE) {
      return {Type::BOOL, constant.type == lexems::Type::TRUE};
    }

    return std::visit([this](const auto& value) -> Constant {
      if constexpr (std::is_same_v<std::decay_t<decltype(value)>, std::monostate>) {
        throw SyntaxError{"Value lexeme should have value"};
      } else {
        return {GetTypeByLexType(Current().type), value};
      }
    }, Current().data);
  }

  void VisitVariable() {
    const auto& id_lex = Current(lexems::Type::ID);

    std::optional<Constant> default_value;
    if (GetNext().type == lexems::Type::ASSIGN) {
      default_value.emplace(GetConstant());
    }

    _visitor.VisitVariable({{std::get<std::string>(id_lex.data)}, std::move(default_value)});
  }

  bool VisitDescription() {
    if (!tu::IsVariableType(Current().type)) {
      return false;
    }
    _visitor.VisitDescription(GetTypeByLexType(Current().type));

    do {
      GetNext();
      VisitVariable();
    } while (Current().type == lexems::Type::COMMA);

    return true;
  }

  void VisitDescriptions() {
    _visitor.VisitDescriptions();

    while (VisitDescription()) {
      Current(lexems::Type::SEMICOLON);
    }
  }

  void VisitOperators() {

  }

private:
  inline const lexems::Lexeme& Validated(const lexems::Lexeme& lexeme, lexems::Type required_type) {
    if (lexeme.type != required_type) {
      throw SyntaxError{"Unexpected Lexeme"};
    }
    return lexeme;
  }

  template<typename TPredicate>
  inline const lexems::Lexeme& Validated(const lexems::Lexeme& lexeme, TPredicate&& predicate) {
    if (!predicate(lexeme.type)) {
      throw SyntaxError{"Unexpected Lexeme"};
    }
    return lexeme;
  }

  inline const lexems::Lexeme& Current() {
    return _lexems[_current];
  }

  inline const lexems::Lexeme& Current(lexems::Type required_type) {
    return Validated(_lexems[_current], required_type);
  }

  template<typename TPredicate>
  inline const lexems::Lexeme& Current(TPredicate&& predicate) {
    return Validated(_lexems[_current], std::forward<TPredicate>(predicate));
  }

  inline const lexems::Lexeme& GetNext() {
    return _lexems[++_current];
  }

  inline const lexems::Lexeme& GetNext(lexems::Type required_type) {
    return Validated(_lexems[++_current], required_type);
  }

  template<typename TPredicate>
  inline const lexems::Lexeme& GetNext(TPredicate&& predicate) {
    return Validated(_lexems[++_current], std::forward<TPredicate>(predicate));
  }

  std::vector<lexems::Lexeme> _lexems;
  std::size_t _current = 0;
  ModelVisitor& _visitor;
};

} // namespace

void VisitCode(std::istream& code, ModelVisitor& visitor) {
  ModelReader{code, visitor}.VisitProgram();
}

} // namespace interpreter
