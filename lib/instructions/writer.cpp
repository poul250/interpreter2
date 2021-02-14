#include "interpreter/instructions/writer.hpp"

#include <functional>

#include "interpreter/ast/types_helpers.hpp"
#include "interpreter/instructions/operations.hpp"

namespace interpreter::instructions {

namespace {

Value ParseAstConstant(ast::VariableType type,
                       std::optional<ast::Constant>&& initial_value) {
  if (initial_value) {
    return std::visit(
        [](auto&& initial) -> Value { return std::move(initial); },
        std::move(initial_value)->value);
  } else {
    return ast::VisitType(
        []<typename T>(utils::TypeTag<T>) -> Value { return T{}; }, type);
  }
}

}  // namespace

void InstructionsWriter::VisitProgram() {}

void InstructionsWriter::VisitDeclarations() {}

void InstructionsWriter::VisitOperators() {}

void InstructionsWriter::VisitVariableDeclaration(
    ast::VariableType type, std::string&& name,
    std::optional<ast::Constant>&& initial_value) {
  auto value = ParseAstConstant(type, std::move(initial_value));
  instructions_.push_back(
      std::make_unique<VariableDefinition>(std::move(name), std::move(value)));
}

void InstructionsWriter::VisitRead(std::string&& name) {
  instructions_.push_back(std::make_unique<Read>(std::move(name)));
}

void InstructionsWriter::VisitWrite() {
  instructions_.push_back(std::make_unique<Write>());
}

void InstructionsWriter::VisitExpressionOperator() {
  instructions_.push_back(std::make_unique<Pop>());
}

void InstructionsWriter::VisitAssign() {
  instructions_.push_back(std::make_unique<BinaryOp<op_type::Assign>>());
}
void InstructionsWriter::VisitOr() {
  // instructions_.push_back(std::make_unique<BinaryOp<std::logical_or<Variable>>>());
}
void InstructionsWriter::VisitAnd() {
  // instructions_.push_back(std::make_unique<BinaryOp<std::logical_and<Variable>>>()
}
void InstructionsWriter::VisitCompare(ast::CompareType) {}
void InstructionsWriter::VisitAdd(ast::AddType add_type) {
  if (add_type == ast::AddType::PLUS) {
    instructions_.push_back(std::make_unique<BinaryOp<op_type::Plus>>());
  }
}
void InstructionsWriter::VisitMul(ast::MulType) {}
void InstructionsWriter::VisitNot() {}

void InstructionsWriter::VisitVariableInvokation(std::string&& variable_name) {
  instructions_.push_back(
      std::make_unique<InvokeVariable>(std::move(variable_name)));
}
void InstructionsWriter::VisitConstantInvokation(ast::Constant&& constant) {
  // TODO: looks wierd, use another structures pls
  instructions_.push_back(std::make_unique<InvokeConstant>(std::visit(
      [](auto&& value) { return Value{value}; }, std::move(constant.value))));
}

}  // namespace interpreter::instructions
