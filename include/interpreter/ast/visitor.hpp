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

enum class CompareType { LT, GT, LE, GE, EQ, NE };

enum class AddType { PLUS, MINUS };

enum class MulType { MUL, DIV, MOD };

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
  virtual void VisitExpressionOperator() = 0;

  // Expression states
  virtual void VisitAssign() = 0;
  virtual void VisitOr() = 0;
  virtual void VisitAnd() = 0;
  virtual void VisitCompare(CompareType compare_type) = 0;
  virtual void VisitAdd(AddType add_type) = 0;
  virtual void VisitMul(MulType mul_type) = 0;
  virtual void VisitNot() = 0;

  virtual void VisitVariableInvokation(std::string&& variable_name) = 0;
  virtual void VisitConstantInvokation(Constant&& constant) = 0;
};

// TODO: move him in another header
void VisitCode(std::istream& code, ModelVisitor& visitor);

}  // namespace interpreter::ast
