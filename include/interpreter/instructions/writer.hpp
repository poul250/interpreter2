#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "instructions.hpp"
#include "interpreter/ast/visitor.hpp"

namespace interpreter::instructions {

struct WriterError : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

class InstructionsWriter : public ast::ModelVisitor {
 public:
  void VisitProgram() override;
  void VisitDeclarations() override;
  void VisitVariableDeclaration(
      ast::VariableType type, std::string&& name,
      std::optional<ast::Constant>&& initial_value = std::nullopt) override;
  void VisitOperators() override;
  void VisitRead(std::string&& name) override;
  void VisitWrite() override;
  void VisitExpressionOperator() override;

  void VisitIf() override;
  void VisitElse() override;
  void VisitEndIf() override;

  void VisitWhile() override;
  void VisitWhileBody() override;
  void VisitEndWhile() override;
  void VisitBreak() override;
  void VisitContinue() override;

  // Expression States
  void VisitAssign() override;
  void VisitOr() override;
  void VisitAnd() override;
  void VisitCompare(ast::CompareType compare_type) override;
  void VisitAdd(ast::AddType add_type) override;
  void VisitMul(ast::MulType mul_type) override;
  void VisitNot() override;
  void VisitVariableInvokation(std::string&& variable_name) override;
  void VisitConstantInvokation(ast::Constant&& constant) override;

  [[nodiscard]] inline const auto& GetInstructions() noexcept {
    return instructions_;
  }

  // pls do something better
  [[nodiscard]] inline InstructionsBlock MakeBlock() noexcept {
    return InstructionsBlock{std::move(instructions_)};
  }

 private:
  std::vector<std::shared_ptr<Instruction>> instructions_;
  std::stack<std::shared_ptr<JumpInstruction>> jump_stack_;
  std::stack<std::vector<std::shared_ptr<JumpInstruction>>> loops_breaks_stack_;
  std::stack<Label> loops_starts_stack_;
};

}  // namespace interpreter::instructions
