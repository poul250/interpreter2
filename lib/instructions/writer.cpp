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
    case Compare::EQ:
      return std::make_shared<BinaryOp<op_type::Equals>>();
    case Compare::NE:
      return std::make_shared<BinaryOp<op_type::NotEquals>>();
    case Compare::LT:
      return std::make_shared<BinaryOp<op_type::Less>>();
    case Compare::GT:
      return std::make_shared<BinaryOp<op_type::Greater>>();
    case Compare::LE:
      return std::make_shared<BinaryOp<op_type::LessOrEq>>();
    case Compare::GE:
      return std::make_shared<BinaryOp<op_type::GreaterOrEq>>();
      // TODO: all operations
  }
  throw WriterError{"Unimplemented mapping for ast::CompareType"};
}

std::shared_ptr<Instruction> MakeAddInstruction(ast::AddType mul_type) {
  // TODO: pls smt smarter
  using Add = ast::AddType;
  switch (mul_type) {
    // waiting for c++20 using enums
    case Add::PLUS:
      return std::make_shared<BinaryOp<op_type::Plus>>();
    case Add::MINUS:
      return std::make_shared<BinaryOp<op_type::Minus>>();
  }
  throw WriterError{"Unimplemented mapping for ast::AddType"};
}

std::shared_ptr<Instruction> MakeMulInstruction(ast::MulType mul_type) {
  // TODO: pls smt smarter
  using Mul = ast::MulType;
  switch (mul_type) {
    // waiting for c++20 using enums
    case Mul::MUL:
      return std::make_shared<BinaryOp<op_type::Mul>>();
    case Mul::DIV:
      return std::make_shared<BinaryOp<op_type::Div>>();
    case Mul::MOD:
      return std::make_shared<BinaryOp<op_type::Mod>>();
      // TODO: all operations
  }
  throw WriterError{"Unimplemented mapping for ast::MulType"};
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

  // jump here from previous jump
  jump_stack_.top()->SetLabel(instructions_.size());
  jump_stack_.pop();

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

  // jump here from previous jump
  jump_stack_.top()->SetLabel(instructions_.size() - 1);
  jump_stack_.pop();
}

void InstructionsWriter::VisitWhile() {
  // store current position on the stack
  loops_starts_stack_.push(instructions_.size() - 1);

  // create list of breaks
  loops_breaks_stack_.push({});
}

void InstructionsWriter::VisitWhileBody() {
  auto jump_to_end = std::make_shared<JumpFalse>();
  // jump to end of loop on false expression
  jump_stack_.push(jump_to_end);
  instructions_.push_back(std::move(jump_to_end));
}

void InstructionsWriter::VisitEndWhile() {
  if (jump_stack_.empty() || loops_breaks_stack_.empty()) {
    throw WriterError{"Missing while block before while end"};
  }
  const auto loop_end_label = instructions_.size();

  jump_stack_.top()->SetLabel(loop_end_label);
  jump_stack_.pop();

  for (const auto& break_jump : loops_breaks_stack_.top()) {
    break_jump->SetLabel(loop_end_label);
  }
  loops_breaks_stack_.pop();

  // add go to loop start instruction
  instructions_.push_back(std::make_shared<GoTo>(loops_starts_stack_.top()));
  loops_starts_stack_.pop();
}

void InstructionsWriter::VisitBreak() {
  if (loops_breaks_stack_.empty()) {
    throw WriterError{"break instruction outside the loop"};
  }
  auto break_jump = std::make_shared<GoTo>();

  // remember break for filling it in the end of loop
  loops_breaks_stack_.top().push_back(break_jump);

  // just put break instruction here
  instructions_.push_back(std::move(break_jump));
}

void InstructionsWriter::VisitContinue() {
  if (loops_starts_stack_.empty()) {
    throw WriterError{"continue instruction outside the loop"};
  }

  // just put break instruction here
  instructions_.push_back(std::make_shared<GoTo>(loops_starts_stack_.top()));
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
  instructions_.push_back(MakeAddInstruction(add_type));
}

void InstructionsWriter::VisitMul(ast::MulType mul_type) {
  instructions_.push_back(MakeMulInstruction(mul_type));
}

void InstructionsWriter::VisitNot() {
  instructions_.push_back(std::make_shared<UnaryOp<op_type::Not>>());
}

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
