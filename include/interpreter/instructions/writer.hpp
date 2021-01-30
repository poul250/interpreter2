#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "instructions.hpp"
#include "interpreter/ast/visitor.hpp"

namespace interpreter::instructions {

class InstructionsWriter : public ast::ModelVisitor {
 public:
  void VisitProgram() override;
  void VisitDeclarations() override;
  void VisitVariableDeclaration(
      ast::VariableType type, std::string&& name,
      std::optional<ast::Constant>&& initial_value = std::nullopt) override;
  void VisitOperators() override;
  void VisitRead(std::string&& name) override;
  // TODO: replace Variable with Expression
  void VisitWrite(std::string&& variable_name) override;

  [[nodiscard]] inline const auto& GetInstructions() noexcept {
    return instructions_;
  }

 private:
  // TODO: should it be std::vector<std::shared_ptr<...>>?
  std::vector<std::unique_ptr<Instruction>> instructions_;
};

}  // namespace interpreter::instructions
