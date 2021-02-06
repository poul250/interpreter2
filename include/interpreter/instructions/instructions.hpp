#pragma once

#include <iosfwd>
#include <memory>
#include <optional>
#include <stack>
#include <unordered_map>
#include <vector>

// TODO: don't include ast/types here, make new models
#include "interpreter/ast/types.hpp"

namespace interpreter::instructions {

struct RuntimeError : public std::runtime_error {
  using runtime_error::runtime_error;
};

struct Variable {
  ast::VariableType type;
  ast::VariableValue value;

  // Variable& operator=(const Variable& other) = default;
  // Variable& operator=(Variable&& other) = default;

  [[nodiscard]] constexpr bool operator==(const Variable& other) const =
      default;
};

struct Literal {
  ast::VariableType type;
  ast::VariableValue value;
};

struct ExecutionContext {
  std::istream& input;
  std::ostream& output;
  std::unordered_map<std::string, Variable> variables;
  // TODO: dont use Variable here
  std::stack<Variable> values_stack;
};

class Instruction {
 public:
  virtual void Execute(ExecutionContext& context) const = 0;
  virtual ~Instruction() = default;
};

class InstructionsBlock : public Instruction {
 public:
  // TODO: should it be shared_ptr?
  inline explicit InstructionsBlock(
      std::vector<std::unique_ptr<Instruction>> instructions)
      : instructions_{std::move(instructions)} {}
  void Execute(ExecutionContext& context) const override;

 private:
  std::vector<std::unique_ptr<Instruction>> instructions_;
};

class VariableDefinition : public Instruction {
 public:
  inline explicit VariableDefinition(
      ast::VariableType type, std::string name,
      std::optional<ast::VariableValue> initial_value) noexcept
      : type_{type},
        name_{std::move(name)},
        initial_value_{std::move(initial_value)} {}
  void Execute(ExecutionContext& context) const override;

 private:
  ast::VariableType type_;
  std::string name_;
  std::optional<ast::VariableValue> initial_value_;
};

class Write : public Instruction {
 public:
  inline explicit Write(std::string variable_name) noexcept
      : variable_name_{std::move(variable_name)} {}
  void Execute(ExecutionContext& context) const override;

 private:
  // TODO: use expression here
  std::string variable_name_;
};

class Read : public Instruction {
 public:
  inline explicit Read(std::string variable_name) noexcept
      : variable_name_{std::move(variable_name)} {}
  void Execute(ExecutionContext& context) const override;

 private:
  std::string variable_name_;
};

class Push : public Instruction {
 public:
  // TODO: dont use Variable
  inline explicit Push(Variable value) noexcept : value_{std::move(value)} {}
  void Execute(ExecutionContext& context) const override;

 private:
  Variable value_;
};

template <typename BinaryOpHandler>
class BinaryOp : Instruction {
  void Execute(ExecutionContext& context) const override;
};

}  // namespace interpreter::instructions

template <typename BinaryOpHandler>
void interpreter::instructions::BinaryOp<BinaryOpHandler>::Execute(
    ExecutionContext& context) const {
  auto& stack = context.values_stack;
  if (stack.size() < 2) {
    throw RuntimeError{"bruh"};
  }

  auto rhs = stack.top();
  stack.pop();

  auto lhs = stack.top();
  stack.pop();

  auto result = BinaryOpHandler{}(std::move(lhs), std::move(rhs));
  stack.push(std::move(result));
}
