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

    VisitDescriptions();
    VisitOperators();

    Current(lexems::Type::CLOSING_BRACE);
  }

  void VisitVariable() {
    const auto& id_lex = GetNext(lexems::Type::ID);
    const auto& after_id_lex = GetNext();

    std::optional<Constant> default_value;
    if (after_id_lex.type == lexems::Type::ASSIGN) {
      // init default value
    }
    _visitor.VisitVariable();
  }

  bool VisitDescription() {
    // TODO: check for correct
    if (!tu::IsVariableType(Current().type)) {
      return false;
    }

    _visitor.VisitDescription(GetTypeByLexType(Current().type));

    do {
      VisitVariable();
    } while (Current().type == lexems::Type::COMMA);

    return true;
  }

  void VisitDescriptions() {
    _visitor.VisitDescriptions();

    while (VisitDescription()) {
      Current(lexems::Type::COMMA);
    }
  }

  void VisitOperators() {

  }


private:
  inline const lexems::Lexeme& Current() {
    return _lexems[_current];
  }

  inline const lexems::Lexeme& Current(lexems::Type required_type) {
    const auto& current_lexeme = _lexems[_current];
    if (required_type != current_lexeme.type) {
      throw SyntaxError{"Unexpected Error"};
    }
    return current_lexeme;
  }

  inline const lexems::Lexeme& GetNext() {
    return _lexems[++_current];
  }

  inline const lexems::Lexeme& GetNext(lexems::Type required_type) {
    const auto& next_lexeme = _lexems[++_current];
    if (required_type != next_lexeme.type) {
      throw SyntaxError{"Unexpected lexeme"};
    }
    return next_lexeme;
  }

  template<typename TPredicate>
  const lexems::Lexeme& GetNext(TPredicate&& predicate) {
    const auto& next_lexeme = _lexems[++_current];
    if (!predicate(next_lexeme.type)) {
      throw SyntaxError{"Unexpected lexeme"};
    }
    return next_lexeme;
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
