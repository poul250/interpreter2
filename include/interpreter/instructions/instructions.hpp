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

using Label = size_t;

struct ExecutionContext {
  // TODO: add reference to parent
  std::istream& input;
  std::ostream& output;
  std::unordered_map<std::string, Value> variables;
  std::stack<OperationValue> values_stack;
  Label current_instruction;
};

class Instruction {
 public:
  virtual void Execute(ExecutionContext& context) const = 0;
  virtual ~Instruction() = default;
};

class NoOp : public Instruction {
 public:
  void Execute(ExecutionContext& context) const;
};

class InstructionsBlock : public Instruction {
 public:
  inline explicit InstructionsBlock(
      std::vector<std::shared_ptr<Instruction>> instructions)
      : instructions_{std::move(instructions)} {}
  void Execute(ExecutionContext& context) const override;

 private:
  std::vector<std::shared_ptr<Instruction>> instructions_;
};

class VariableDefinition : public Instruction {
 public:
  inline explicit VariableDefinition(std::string name,
                                     Value initial_value) noexcept
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
  inline explicit Read(std::string variable_name) noexcept
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
  inline explicit InvokeConstant(Value value) noexcept
      : value_{std::move(value)} {}
  void Execute(ExecutionContext& context) const override;

 private:
  Value value_;
};

class InvokeVariable : public Instruction {
 public:
  inline explicit InvokeVariable(std::string name) noexcept
      : name_{std::move(name)} {}
  void Execute(ExecutionContext& context) const override;

 private:
  std::string name_;
};

class JumpInstruction : public Instruction {
 public:
  inline explicit JumpInstruction(Label label) noexcept : label_{label} {}
  inline void SetLabel(Label label) noexcept { label_ = label; }

 protected:
  Label label_;
};

class GoTo : public JumpInstruction {
 public:
  inline explicit GoTo(Label label = 0) noexcept : JumpInstruction{label} {}
  void Execute(ExecutionContext& context) const override;
};

class JumpBool : public JumpInstruction {
 public:
  inline explicit JumpBool(bool jump_statement, Label label = 0) noexcept
      : JumpInstruction{label}, jump_statement_{jump_statement} {}
  void Execute(ExecutionContext& context) const override;

 private:
  bool jump_statement_;
};

class JumpFalse : public JumpBool {
 public:
  inline explicit JumpFalse(Label label = 0) noexcept
      : JumpBool{false, label} {}
};

class JumpTrue : public JumpBool {
 public:
  inline explicit JumpTrue(Label label = 0) noexcept : JumpBool{true, label} {}
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
    throw RuntimeError{"Error, no operands for binary expression"};
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
  auto& stack = context.values_stack;
  if (stack.size() < 1) {
    // TODO: something smarter pls
    throw RuntimeError{"Error: no operands for unary expression"};
  }

  auto value = stack.top();
  stack.pop();

  stack.push(PerformOperation<UnaryOpHandler>(std::move(value)));
}
