#pragma once

#include <iosfwd>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

#include "interpreter/syntax/types.hpp"

namespace interpreter::instructions {

struct RuntimeError : public std::runtime_error {
  using runtime_error::runtime_error;
};

struct Variable {
  syntax::VariableType type;
  std::optional<syntax::VariableValue> value;

  Variable& operator=(const Variable& other) = default;
  Variable& operator=(Variable&& other) = default;

  [[nodiscard]] bool operator==(const Variable& other) const = default;
};

struct ExecutionContext {
  std::istream& input;
  std::ostream& output;
  std::unordered_map<std::string, Variable> variables;
};

class Instruction {
 public:
  virtual void Execute(ExecutionContext& context) const = 0;
  virtual ~Instruction() = default;
};

class InstructionsBlock : public Instruction {
 public:
  explicit InstructionsBlock(
      std::vector<std::unique_ptr<Instruction>> instructions_) noexcept;
  void Execute(ExecutionContext& context) const override;

 private:
  std::vector<std::unique_ptr<Instruction>> instructions_;
};

class VariableDefinition : public Instruction {
 public:
  explicit VariableDefinition(
      syntax::VariableType type, std::string name,
      std::optional<syntax::Constant> initial_value) noexcept;
  void Execute(ExecutionContext& context) const override;

 private:
  syntax::VariableType type_;
  std::string name_;
  std::optional<syntax::Constant> initial_value_;
};

class Write : public Instruction {
 public:
  explicit Write(std::string variable_name) noexcept;
  void Execute(ExecutionContext& context) const override;

 private:
  // TODO: use expression here
  std::string variable_name_;
};

class Read : public Instruction {
 public:
  explicit Read(std::string variable_name) noexcept;
  void Execute(ExecutionContext& context) const override;

 private:
  std::string variable_name_;
};

}  // namespace interpreter::instructions
