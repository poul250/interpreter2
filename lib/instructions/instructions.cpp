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

ExecutionContext MakeChildExecutionContext(const ExecutionContext& parent) {
  return ExecutionContext{.input = parent.input,
                          .output = parent.output,
                          .variables = {},
                          .values_stack = {},
                          .current_instruction = 0};
}

}  // namespace

void NoOp::Execute(ExecutionContext& context) const {}

void InstructionsBlock::Execute(ExecutionContext& parent_context) const {
  auto context = MakeChildExecutionContext(parent_context);
  while (context.current_instruction < instructions_.size()) {
    instructions_[context.current_instruction]->Execute(context);
    ++context.current_instruction;
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

void JumpBool::Execute(ExecutionContext& context) const {
  if (context.values_stack.empty()) {
    throw RuntimeError{"No expressions for perform bool jump"};
  }

  auto value = context.values_stack.top();
  context.values_stack.pop();

  const bool bool_on_stack =
      VisitOperationValues(ToBoolVisitor{}, std::move(value));
  if (bool_on_stack == jump_statement_) {
    context.current_instruction = label_;
  }
}

void GoTo::Execute(ExecutionContext& context) const {
  context.current_instruction = label_;
}

}  // namespace interpreter::instructions
