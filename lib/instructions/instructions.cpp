#include "interpreter/instructions/instructions.hpp"

#include <iostream>

#include "interpreter/utils/format.hpp"

namespace interpreter::instructions {

namespace {

template <typename T>
[[nodiscard]] T ReadValue(std::istream& input) {
  T value;
  input >> value;
  return value;
}

}  // namespace

InstructionsBlock::InstructionsBlock(
    std::vector<std::unique_ptr<Instruction>> instructions) noexcept
    : instructions_(std::move(instructions)) {}

void InstructionsBlock::Execute(ExecutionContext& context) const {
  for (const auto& instruction : instructions_) {
    instruction->Execute(context);
  }
}

VariableDefinition::VariableDefinition(
    syntax::VariableType type, std::string name,
    std::optional<syntax::Constant> initial_value) noexcept
    : type_(type),
      name_(std::move(name)),
      initial_value_(std::move(initial_value)) {}

void VariableDefinition::Execute(ExecutionContext& context) const {
  if (context.variables.contains(name_)) {
    throw RuntimeError{
        utils::format("Variable {} is already declared.", name_)};
  }

  if (initial_value_ != std::nullopt) {
    const auto& constant = *initial_value_;
    std::visit(syntax::TypeChecker{type_}, constant.value);
    context.variables[name_] = Variable{type_, constant.value};
  } else {
    context.variables[name_] = Variable{type_, std::nullopt};
  }
}

Write::Write(std::string variable_name) noexcept
    : variable_name_(std::move(variable_name)) {}

void Write::Execute(ExecutionContext& context) const {
  auto var_it = context.variables.find(variable_name_);
  if (var_it == context.variables.cend()) [[unlikely]] {
      throw RuntimeError{
          utils::format("Failed to write variable '{}', it is not declared.",
                        variable_name_)};
    }

  const auto& variable = var_it->second;
  const auto& opt_value = variable.value;
  if (opt_value == std::nullopt) [[unlikely]] {
      throw RuntimeError{utils::format(
          "Failed to write variable '{}', it is not defined.", variable_name_)};
    }

  const auto& value = *opt_value;
  std::visit([&context](const auto& value) { context.output << value; }, value);
}

Read::Read(std::string variable_name) noexcept
    : variable_name_(std::move(variable_name)) {}

void Read::Execute(ExecutionContext& context) const {
  auto var_it = context.variables.find(variable_name_);
  if (var_it == context.variables.cend()) {
    throw RuntimeError{utils::format(
        "Failed to read variable '{}', it is not declared.", variable_name_)};
  }

  auto& variable = var_it->second;
  auto& opt_value = variable.value;
  if (opt_value != std::nullopt) {
    std::visit([&context](auto& value) { context.input >> value; }, *opt_value);
  } else {
    variable.value = syntax::VisitType(variable.type)(
        [&context]<typename T>(utils::TypeTag<T>) -> syntax::VariableValue {
          T value;
          context.input >> value;
          return value;
        });
  }
}

}  // namespace interpreter::instructions
