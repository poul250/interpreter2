#include "interpreter/instructions/instructions.hpp"

#include <iostream>

#include "interpreter/instructions/operations.hpp"
#include "interpreter/utils/format.hpp"

namespace interpreter::instructions {

namespace {

struct Writer {
  std::ostream& output;

  template <typename T>
  void operator()(const T& t) {
    output << t;
  }

  template <typename T>
  void operator()(std::reference_wrapper<T> t) {
    output << t.get();
  }
};

}  // namespace

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

  context.variables[name_] = initial_value_;
}

void Write::Execute(ExecutionContext& context) const {
  auto& stack = context.values_stack;
  if (stack.empty()) {
    throw RuntimeError{"Nothing to write"};
  }

  const auto value = stack.top();
  stack.pop();

  VisitOperationValues(Writer{context.output}, value);
}

void Read::Execute(ExecutionContext& context) const {
  auto var_it = context.variables.find(variable_name_);
  if (var_it == context.variables.cend()) {
    throw RuntimeError{utils::format(
        "Failed to read variable '{}', it is not declared.", variable_name_)};
  }

  auto& variable = var_it->second;
  VisitValues([&context](auto& value) { context.input >> value; }, variable);
}

void Pop::Execute(ExecutionContext& context) const {
  if (context.values_stack.empty()) {
    throw RuntimeError{"Empty expression error"};
  }
  context.values_stack.pop();
}

void InvokeConstant::Execute(ExecutionContext& context) const {
  context.values_stack.push(value_);
}

void InvokeVariable::Execute(ExecutionContext& context) const {
  if (!context.variables.contains(name_)) {
    throw RuntimeError{utils::format("Variable {} is not defined", name_)};
  }
  auto& variable = context.variables.at(name_);
  context.values_stack.push(VisitValues(
      [](auto& value) -> OperationValue { return Reference{std::ref(value)}; },
      variable));
}

}  // namespace interpreter::instructions
