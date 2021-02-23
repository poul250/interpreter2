#pragma once

#include <gmock/gmock.h>

#include "interpreter/ast/visitor.hpp"

namespace interpreter::ast {

class MockModelVisitor : public ModelVisitor {
 public:
  MOCK_METHOD(void, VisitProgram, (), (override));
  MOCK_METHOD(void, VisitDeclarations, (), (override));
  MOCK_METHOD(void, VisitVariableDeclaration,
              (VariableType type, std::string&& name,
               std::optional<Constant>&& initial_value),
              (override));
  MOCK_METHOD(void, VisitOperators, (), (override));
  MOCK_METHOD(void, VisitRead, (std::string && name), (override));
  MOCK_METHOD(void, VisitWrite, (), (override));
  MOCK_METHOD(void, VisitExpressionOperator, (), (override));

  MOCK_METHOD(void, VisitIf, (), (override));
  MOCK_METHOD(void, VisitElse, (), (override));
  MOCK_METHOD(void, VisitEndIf, (), (override));

  MOCK_METHOD(void, VisitWhile, (), (override));
  MOCK_METHOD(void, VisitWhileBody, (), (override));
  MOCK_METHOD(void, VisitEndWhile, (), (override));

  MOCK_METHOD(void, VisitDoWhile, (), (override));
  MOCK_METHOD(void, VisitDoWhileEnd, (), (override));

  MOCK_METHOD(void, VisitBreak, (), (override));
  MOCK_METHOD(void, VisitContinue, (), (override));

  MOCK_METHOD(void, VisitAssign, (), (override));
  MOCK_METHOD(void, VisitOr, (), (override));
  MOCK_METHOD(void, VisitAnd, (), (override));
  MOCK_METHOD(void, VisitCompare, (CompareType compare_type), (override));
  MOCK_METHOD(void, VisitAdd, (AddType add_type), (override));
  MOCK_METHOD(void, VisitMul, (MulType mul_type), (override));
  MOCK_METHOD(void, VisitNot, (), (override));
  MOCK_METHOD(void, VisitVariableInvokation, (std::string && variable_name),
              (override));
  MOCK_METHOD(void, VisitConstantInvokation, (Constant && constant),
              (override));
};

}  // namespace interpreter::ast
