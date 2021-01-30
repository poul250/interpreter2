#pragma once

#include <memory>
#include <vector>

#include "instructions.hpp"
#include "interpreter/ast/visitor.hpp"

namespace interpreter::instructions {

class InstructionsWriter : public ast::ModelVisitor {
 public:
  void VisitProgram() override;
  void VisitDescriptions() override;
  void VisitVariableDeclaration(
      ast::VariableType type, std::string&& name,
      std::optional<ast::Constant>&& initial_value = std::nullopt) override;
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
