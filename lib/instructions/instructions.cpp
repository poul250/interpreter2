#include "interpreter/instructions/instructions.hpp"

#include <iostream>

#include "interpreter/ast/types.hpp"
#include "interpreter/ast/types_helpers.hpp"
#include "interpreter/utils/format.hpp"

namespace interpreter::instructions {

void InstructionsBlock::Execute(ExecutionContext& context) const {
  for (const auto& instruction : instructions_) {
    instruction->Execute(context);
  }
}

void VariableDefinition::Execute(ExecutionContext& context) const {
  if (context.variables.contains(name_)) {
    throw RuntimeError{
        utils::format("Variable {} is already declared.", name_)};
  }

  if (initial_value_ != std::nullopt) {
    const auto& value = *initial_value_;
    std::visit(ast::TypeChecker{type_}, value);
    context.variables[name_] = Variable{type_, value};
  } else {
    context.variables[name_] = Variable{
        type_, ast::VisitType(
                   []<typename T>(utils::TypeTag<T>) -> ast::VariableValue {
                     // init by default value
                     return T{};
                   },
                   type_)};
  }
}

void Write::Execute(ExecutionContext& context) const {
  auto var_it = context.variables.find(variable_name_);
  if (var_it == context.variables.cend()) [[unlikely]] {
      throw RuntimeError{
          utils::format("Failed to write variable '{}', it is not declared.",
                        variable_name_)};
    }

  const auto& variable = var_it->second;
  std::visit([&context](const auto& value) { context.output << value; },
             variable.value);
}

void Read::Execute(ExecutionContext& context) const {
  auto var_it = context.variables.find(variable_name_);
  if (var_it == context.variables.cend()) {
    throw RuntimeError{utils::format(
        "Failed to read variable '{}', it is not declared.", variable_name_)};
  }

  auto& variable = var_it->second;
  std::visit([&context](auto& value) { context.input >> value; },
             variable.value);
}

void Push::Execute(ExecutionContext& context) const {
  context.values_stack.push(value_);
}

}  // namespace interpreter::instructions
