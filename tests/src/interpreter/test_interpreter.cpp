#include "test_interpreter.hpp"

#include <gtest/gtest.h>

namespace interpreter::test {

TEST(TestInterpreter, HelloWorld) {
  const auto program = R"abc(
    program {
      write("Hello world!");
    }
  )abc";
  ASSERT_EQ(RunInterpreter(program), "Hello world!");
}

TEST(TestInterpreter, TestConditional) {
  const auto program = R"abc(
    program {
        int x;

        read(x);

        if (x > 20) {
            write("Greater then 20");
        } else if (x > 10) {
            write("Greater then 10");
            if (x < 15) {
                write(" and less then 15");
            } else {
                write(" and greater or equals to 15");
            }
        } else {
            write("Less then 10");
        }
    }
  )abc";

  ASSERT_EQ(RunInterpreter(program, "25"), "Greater then 20");
  ASSERT_EQ(RunInterpreter(program, "18"),
            "Greater then 10 and greater or equals to 15");
  ASSERT_EQ(RunInterpreter(program, "12"), "Greater then 10 and less then 15");
  ASSERT_EQ(RunInterpreter(program, "9"), "Less then 10");
}

TEST(TestInterpreter, TestExpressions) {
  const auto program = R"abc(
    program {
        int x, y;
        read(x);
        read(y);
        write(x + y, "123", "456" + "00", "\n");

        x = 20;
        write(x, x = 10);
    }
  )abc";
  const auto input = "1 2";
  const auto output = "312345600\n2010";
  ASSERT_EQ(RunInterpreter(program, input), output);
}

TEST(TestInterpreter, DoWhile) {
  const auto program = R"abc(
    program {
        int x = 10;

        do {
            write(x, "\n");
            x = x - 1;
        } while(x >= 0);
    }
  )abc";
  ASSERT_EQ(RunInterpreter(program), "10\n9\n8\n7\n6\n5\n4\n3\n2\n1\n0\n");
}

}  // namespace interpreter::test
