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

std::shared_ptr<Instruction> MakeCompareInstruction(
    ast::CompareType compare_type) {
  // TODO: pls smt smarter
  using Compare = ast::CompareType;
  switch (compare_type) {
    // waiting for c++20 using enums
    case Compare::LT:
      return std::make_shared<BinaryOp<op_type::Less>>();
    case Compare::GT:
      return std::make_shared<BinaryOp<op_type::Greater>>();
      // TODO: all operations
  }
  throw WriterError{"Unimplemented mapping for ast::CompareType"};
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
      std::make_shared<VariableDefinition>(std::move(name), std::move(value)));
}

void InstructionsWriter::VisitRead(std::string&& name) {
  instructions_.push_back(std::make_shared<Read>(std::move(name)));
}

void InstructionsWriter::VisitWrite() {
  instructions_.push_back(std::make_shared<Write>());
}

void InstructionsWriter::VisitExpressionOperator() {
  instructions_.push_back(std::make_shared<Pop>());
}

void InstructionsWriter::VisitIf() {
  auto jump = std::make_shared<JumpFalse>();
  // remember this jump
  jump_stack_.push(jump);
  // just add jump instruction in instructions list
  instructions_.push_back(std::move(jump));
}
void InstructionsWriter::VisitElse() {
  if (jump_stack_.empty()) {
    throw WriterError{"Missing if block before else"};
  }

  // take previous jump instruction
  auto prev_jump_instruction = std::move(jump_stack_.top());
  jump_stack_.pop();

  // jump here from previous jump
  prev_jump_instruction->SetLabel(instructions_.size());

  auto jump = std::make_shared<GoTo>();
  // remember this point
  jump_stack_.push(jump);
  // just add jump instruction in instructions list
  instructions_.push_back(std::move(jump));
}
void InstructionsWriter::VisitEndIf() {
  if (jump_stack_.empty()) {
    throw WriterError{"Missing if block before endif"};
  }

  // take previous jump instruction
  auto prev_jump_instruction = std::move(jump_stack_.top());
  jump_stack_.pop();

  // TODO: is it ok, that we are doing size_t - 1?
  // jump here from previous jump
  prev_jump_instruction->SetLabel(instructions_.size() - 1);
}

void InstructionsWriter::VisitAssign() {
  instructions_.push_back(std::make_shared<BinaryOp<op_type::Assign>>());
}

void InstructionsWriter::VisitOr() {
  instructions_.push_back(std::make_shared<BinaryOp<op_type::Or>>());
}

void InstructionsWriter::VisitAnd() {
  instructions_.push_back(std::make_shared<BinaryOp<op_type::And>>());
}

void InstructionsWriter::VisitCompare(ast::CompareType compare_type) {
  instructions_.push_back(MakeCompareInstruction(compare_type));
}

void InstructionsWriter::VisitAdd(ast::AddType add_type) {
  if (add_type == ast::AddType::PLUS) {
    instructions_.push_back(std::make_shared<BinaryOp<op_type::Plus>>());
  }
}

void InstructionsWriter::VisitMul(ast::MulType) {}

void InstructionsWriter::VisitNot() {}

void InstructionsWriter::VisitVariableInvokation(std::string&& variable_name) {
  instructions_.push_back(
      std::make_shared<InvokeVariable>(std::move(variable_name)));
}

void InstructionsWriter::VisitConstantInvokation(ast::Constant&& constant) {
  // TODO: looks wierd, use another structures pls
  instructions_.push_back(std::make_shared<InvokeConstant>(std::visit(
      [](auto&& value) { return Value{value}; }, std::move(constant.value))));
}

}  // namespace interpreter::instructions
