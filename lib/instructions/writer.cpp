#include "interpreter/instructions/writer.hpp"

namespace interpreter::instructions {

void InstructionsWriter::VisitProgram() {}

void InstructionsWriter::VisitDeclarations() {}

void InstructionsWriter::VisitOperators() {}

void InstructionsWriter::VisitVariableDeclaration(
    ast::VariableType type, std::string&& name,
    std::optional<ast::Constant>&& initial_value) {
  // TODO: looks wierd
  std::optional<ast::VariableValue> value;
  if (initial_value != std::nullopt) {
    value = std::move(initial_value->value);
  }
  instructions_.push_back(std::make_unique<VariableDefinition>(
      VariableDefinition{type, std::move(name), std::move(value)}));
}

void InstructionsWriter::VisitRead(std::string&& name) {
  instructions_.push_back(std::make_unique<Read>(Read{std::move(name)}));
}

void InstructionsWriter::VisitWrite(std::string&& name) {
  instructions_.push_back(std::make_unique<Write>(Write{std::move(name)}));
}

void InstructionsWriter::VisitExpressionOperator() {}

void InstructionsWriter::VisitAssign() {}
void InstructionsWriter::VisitOr() {}
void InstructionsWriter::VisitAnd() {}
void InstructionsWriter::VisitCompare(ast::CompareType) {}
void InstructionsWriter::VisitAdd(ast::AddType) {}
void InstructionsWriter::VisitMul(ast::MulType) {}
void InstructionsWriter::VisitNot() {}

void InstructionsWriter::VisitVariableInvokation(std::string&& variable_name) {}
void InstructionsWriter::VisitConstantInvokation(ast::Constant&& constant) {}

}  // namespace interpreter::instructions
