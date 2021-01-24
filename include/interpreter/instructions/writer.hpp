#pragma once

#include <memory>
#include <vector>

#include "instructions.hpp"
#include "interpreter/syntax/visitor.hpp"

namespace interpreter::instructions {

class InstructionsWriter : public syntax::ModelVisitor {
 public:
  void VisitProgram() override;
  void VisitDescriptions() override;
  void VisitVariableDeclaration(
      syntax::VariableType type, std::string&& name,
      std::optional<syntax::Constant>&& initial_value = std::nullopt) override;
  void VisitRead(std::string&& name) override;
  void VisitOperators() override;

  // TODO: replace Variable with Expression
  void VisitWrite(std::string&& variable_name) override;

  [[nodiscard]] inline const std::vector<std::unique_ptr<Instruction>>&
  GetInstructions() noexcept {
    return instructions_;
  }

 private:
  std::vector<std::unique_ptr<Instruction>> instructions_;
};

}  // namespace interpreter::instructions
