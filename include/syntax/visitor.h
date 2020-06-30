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

class Identifier {
};

class Constant {
};

class ModelVisitor {
public:
  virtual ~ModelVisitor() = 0;

  virtual void VisitProgram() = 0;
  virtual void VisitDescriptions() = 0;
  virtual void VisitDescription(Type type) = 0;
  virtual void VisitType(Type&& type) = 0;
  virtual void VisitVariable() = 0;
  virtual void VisitRead(Identifier identifier) = 0;

  // TODO: replace Identifier with Expression
  virtual void VisitWrite(Identifier identifier) = 0;
};

void VisitCode(std::istream& code, ModelVisitor& visitor);

} // namespace intepreter::syntax
