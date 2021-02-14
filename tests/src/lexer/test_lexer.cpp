#include <gtest/gtest.h>

#include <sstream>

#include "interpreter/lexer/lexer.hpp"

namespace test {

using namespace interpreter::lexer;

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

TEST(TestLexer, TestEmpty) { MakeTestLexer("", {{LexType::NONE}}); }

TEST(TestLexer, SimpleTest) {
  MakeTestLexer(R"(program{})",  //
                {{LexType::PROGRAM},
                 {LexType::OPENING_BRACE},
                 {LexType::CLOSING_BRACE},
                 {LexType::NONE}});
}

TEST(TestLexer, TestSpaces) {
  MakeTestLexer(
      R"(
         program   {
    }   )",
      {{LexType::PROGRAM},
       {LexType::OPENING_BRACE},
       {LexType::CLOSING_BRACE},
       {LexType::NONE}});
}

TEST(TestLexer, EscapeSymbols) {
  MakeTestLexer(R"(  "a\b" "a\n" "a\t" "a\"" )",  //
                {{LexType::VALUE_STR, "ab"},
                 {LexType::VALUE_STR, "a\n"},
                 {LexType::VALUE_STR, "a\t"},
                 {LexType::VALUE_STR, "a\""},
                 {LexType::NONE}});
}

TEST(TestLexer, TestDeclarations) {
  MakeTestLexer(
      R"(
      int x, y =10;
    )",
      {{LexType::TYPE_INT},
       {LexType::ID, "x"},
       {LexType::COMMA},
       {LexType::ID, "y"},
       {LexType::ASSIGN},
       {LexType::VALUE_INT, 10},
       {LexType::SEMICOLON},
       {LexType::NONE}});

  MakeTestLexer(
      R"(
        real abc= 123.4; 
        int x = 54321 ;
      )",
      {{LexType::TYPE_REAL},
       {LexType::ID, "abc"},
       {LexType::ASSIGN},
       {LexType::VALUE_REAL, 123.4},
       {LexType::SEMICOLON},
       {LexType::TYPE_INT},
       {LexType::ID, "x"},
       {LexType::ASSIGN},
       {LexType::VALUE_INT, 54321},
       {LexType::SEMICOLON},
       {LexType::NONE}});
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
