#include <gtest/gtest.h>

#include <sstream>

#include "interpreter/lex/lexems.hpp"
#include "interpreter/lex/types.hpp"

namespace test {

using namespace interpreter::lexems;

namespace {

void MakeTestLexer(const std::string& program,
                   const std::vector<Lexeme>& expected_lexems) {
  std::istringstream input_stream{program};

  std::vector<Lexeme> given_lexems;
  for (auto lex : ParseLexems(input_stream)) {
    given_lexems.push_back(std::move(lex));
  }

  ASSERT_EQ(given_lexems, expected_lexems);
}

void JustParse(const std::string& program) {
  std::istringstream input_stream{program};
  for (const auto& _ : ParseLexems(input_stream)) {
    ;
  }
}

}  // namespace

TEST(TestLexer, TestEmpty) { MakeTestLexer("", {{Type::NONE}}); }

TEST(TestLexer, SimpleTest) {
  MakeTestLexer(R"(program{})",  //
                {{Type::PROGRAM},
                 {Type::OPENING_BRACE},
                 {Type::CLOSING_BRACE},
                 {Type::NONE}});
}

TEST(TestLexer, TestSpaces) {
  MakeTestLexer(
      R"(
         program   {
    }   )",
      {{Type::PROGRAM},
       {Type::OPENING_BRACE},
       {Type::CLOSING_BRACE},
       {Type::NONE}});
}

TEST(TestLexer, TestDeclarations) {
  MakeTestLexer(
      R"(
      int x, y =10;
    )",
      {{Type::TYPE_INT},
       {Type::ID, "x"},
       {Type::COMMA},
       {Type::ID, "y"},
       {Type::ASSIGN},
       {Type::VALUE_INT, 10},
       {Type::SEMICOLON},
       {Type::NONE}});

  MakeTestLexer(
      R"(
        real abc= 123.4; 
        int x = 54321 ;
      )",
      {{Type::TYPE_REAL},
       {Type::ID, "abc"},
       {Type::ASSIGN},
       {Type::VALUE_REAL, 123.4},
       {Type::SEMICOLON},
       {Type::TYPE_INT},
       {Type::ID, "x"},
       {Type::ASSIGN},
       {Type::VALUE_INT, 54321},
       {Type::SEMICOLON},
       {Type::NONE}});
}

TEST(TestLexer, UnexpectedSymbols) {
  ASSERT_THROW(JustParse("@"), LexicalError);
  ASSERT_THROW(JustParse("#"), LexicalError);
  ASSERT_THROW(JustParse("$"), LexicalError);

  ASSERT_NO_THROW(JustParse("123"));
  ASSERT_NO_THROW(JustParse("abc"));
  ASSERT_NO_THROW(JustParse(R"(a "123")"));

  ASSERT_NO_THROW(JustParse("!="));
  ASSERT_THROW(JustParse("!"), LexicalError);
  ASSERT_THROW(JustParse("!>"), LexicalError);
}

}  // namespace test
