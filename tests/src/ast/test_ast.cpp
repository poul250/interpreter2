#include <gtest/gtest.h>

#include <sstream>

#include "interpreter/ast/visitor.hpp"
#include "mock_model_visitor.hpp"

using namespace interpreter::ast;

namespace test {
TEST(TestAst, TestEmpty) {
  std::stringstream code{"program {}"};

  MockModelVisitor visitor;

  EXPECT_CALL(visitor, VisitProgram()).Times(1);
  EXPECT_CALL(visitor, VisitDeclarations()).Times(1);
  EXPECT_CALL(visitor, VisitOperators()).Times(1);

  VisitCode(code, visitor);
}

}  // namespace test
