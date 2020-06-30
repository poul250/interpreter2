#pragma once
#include <vector>
#include <variant>
#include <optional>

#include <lex/lexems.h>

namespace interpreter::syntax {

class SyntaxError : public std::runtime_error {
public:
  inline explicit SyntaxError(const std::string& error) : std::runtime_error{error} {
  }
};

enum class Type {
  INT,
  REAL,
  STR,
  BOOL,
};

struct Identifier {
  const std::string name;
};

struct Constant {
  const Type type;
  const std::variant<int, double, bool, std::string> value;
};

struct Variable {
  Identifier id;
  std::optional<Constant> default_value;
};


class ModelVisitor {
public:
  virtual ~ModelVisitor() = 0;

  virtual void VisitProgram() = 0;
  virtual void VisitDescriptions() = 0;
  virtual void VisitDescription(Type type) = 0;
  virtual void VisitVariable(Variable&& variable) = 0;
  virtual void VisitRead(Identifier identifier) = 0;

  // TODO: replace Identifier with Expression
  virtual void VisitWrite(Identifier identifier) = 0;
};

void VisitCode(std::istream& code, ModelVisitor& visitor);

} // namespace intepreter::syntax
