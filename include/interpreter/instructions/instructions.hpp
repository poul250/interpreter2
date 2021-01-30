#pragma once

#include <iosfwd>
#include <memory>
#include <optional>
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
  std::optional<ast::VariableValue> value;

  Variable& operator=(const Variable& other) = default;
  Variable& operator=(Variable&& other) = default;

  [[nodiscard]] constexpr bool operator==(const Variable& other) const =
      default;
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
      std::optional<ast::Constant> initial_value) noexcept
      : type_{type},
        name_{std::move(name)},
        initial_value_{std::move(initial_value)} {}
  void Execute(ExecutionContext& context) const override;

 private:
  ast::VariableType type_;
  std::string name_;
  std::optional<ast::Constant> initial_value_;
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

}  // namespace interpreter::instructions
