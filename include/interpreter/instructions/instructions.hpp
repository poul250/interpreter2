#pragma once

#include <iosfwd>
#include <memory>
#include <optional>
#include <stack>
#include <unordered_map>
#include <vector>

#include "operations.hpp"

namespace interpreter::instructions {

struct RuntimeError : public std::runtime_error {
  using runtime_error::runtime_error;
};

struct ExecutionContext {
  std::istream& input;
  std::ostream& output;
  std::unordered_map<std::string, Value> variables;
  std::stack<OperationValue> values_stack;
};

class Instruction {
 public:
  virtual void Execute(ExecutionContext& context) const = 0;
  virtual ~Instruction() = default;
};

class InstructionsBlock : public Instruction {
 public:
  // TODO: should it be shared_ptr?
  inline InstructionsBlock(
      std::vector<std::unique_ptr<Instruction>> instructions)
      : instructions_{std::move(instructions)} {}
  void Execute(ExecutionContext& context) const override;

 private:
  std::vector<std::unique_ptr<Instruction>> instructions_;
};

class VariableDefinition : public Instruction {
 public:
  inline VariableDefinition(std::string name, Value initial_value) noexcept
      : name_{std::move(name)}, initial_value_{std::move(initial_value)} {}
  void Execute(ExecutionContext& context) const override;

 private:
  std::string name_;
  Value initial_value_;
};

class Write : public Instruction {
 public:
  void Execute(ExecutionContext& context) const override;
};

class Read : public Instruction {
 public:
  inline Read(std::string variable_name) noexcept
      : variable_name_{std::move(variable_name)} {}
  void Execute(ExecutionContext& context) const override;

 private:
  std::string variable_name_;
};

class Pop : public Instruction {
 public:
  void Execute(ExecutionContext& context) const override;
};

class InvokeConstant : public Instruction {
 public:
  // TODO: dont use Variable
  inline InvokeConstant(Value value) noexcept : value_{std::move(value)} {}
  void Execute(ExecutionContext& context) const override;

 private:
  Value value_;
};

class InvokeVariable : public Instruction {
 public:
  inline InvokeVariable(std::string name) noexcept : name_{std::move(name)} {}
  void Execute(ExecutionContext& context) const override;

 private:
  std::string name_;
};

template <typename BinaryOpHandler>
class BinaryOp : public Instruction {
 public:
  void Execute(ExecutionContext& context) const override;
};

template <typename UnaryOpHandler>
class UnaryOp : public Instruction {
 public:
  void Execute(ExecutionContext& context) const override;
};

}  // namespace interpreter::instructions

template <typename BinaryOpHandler>
void interpreter::instructions::BinaryOp<BinaryOpHandler>::Execute(
    ExecutionContext& context) const {
  auto& stack = context.values_stack;
  if (stack.size() < 2) {
    // TODO: something smarter pls
    throw RuntimeError{"bruh"};
  }

  auto rhs = stack.top();
  stack.pop();

  auto lhs = stack.top();
  stack.pop();

  stack.push(PerformOperation<BinaryOpHandler>(std::move(lhs), std::move(rhs)));
}

template <typename UnaryOpHandler>
void interpreter::instructions::UnaryOp<UnaryOpHandler>::Execute(
    ExecutionContext& context) const {
  auto stack = context.values_stack;
  if (stack.size() < 1) {
    // TODO: something smarter pls
    throw RuntimeError{"bruh"};
  }

  auto value = stack.top();
  stack.pop();

  stack.push(UnaryOpHandler{}(std::move(value)));
}
