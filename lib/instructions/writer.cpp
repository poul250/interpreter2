#include "interpreter/instructions/writer.hpp"

namespace interpreter::instructions {

void InstructionsWriter::VisitProgram() {}

void InstructionsWriter::VisitDescriptions() {}

void InstructionsWriter::VisitOperators() {}

void InstructionsWriter::VisitVariableDeclaration(
    ast::VariableType type, std::string&& name,
    std::optional<ast::Constant>&& initial_value) {
  instructions_.push_back(std::make_unique<VariableDefinition>(
      VariableDefinition{type, std::move(name), std::move(initial_value)}));
}

void InstructionsWriter::VisitRead(std::string&& name) {
  instructions_.push_back(std::make_unique<Read>(Read{std::move(name)}));
}

void InstructionsWriter::VisitWrite(std::string&& name) {
  instructions_.push_back(std::make_unique<Write>(Write{std::move(name)}));
}

}  // namespace interpreter::instructions
