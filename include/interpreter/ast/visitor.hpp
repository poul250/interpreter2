#pragma once

#include <iosfwd>
#include <optional>
#include <stdexcept>
#include <string>

#include "types.hpp"

namespace interpreter::ast {

struct SyntaxError : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

struct ParseExpressionError : public SyntaxError {
  using SyntaxError::SyntaxError;
};

struct Constant {
  VariableType type;
  VariableValue value;
};

class ModelVisitor {
 public:
  virtual ~ModelVisitor() = default;

  virtual void VisitProgram() = 0;
  virtual void VisitDeclarations() = 0;
  virtual void VisitVariableDeclaration(
      VariableType type, std::string&& name,
      std::optional<Constant>&& initial_value = std::nullopt) = 0;
  virtual void VisitOperators() = 0;

  virtual void VisitRead(std::string&& name) = 0;
  // TODO: replace variable_name with Expression
  virtual void VisitWrite(std::string&& variable_name) = 0;
};

// TODO: move him in another header
void VisitCode(std::istream& code, ModelVisitor& visitor);

}  // namespace interpreter::ast
