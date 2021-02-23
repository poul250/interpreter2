#pragma once

#include <sstream>
#include <string>

#include "interpreter/instructions/writer.hpp"

namespace interpreter::test {

inline std::string RunInterpreter(const std::string& code,
                                  const std::string& input = "") {
  std::istringstream code_stream{code};
  std::istringstream input_stream{input};
  std::ostringstream output_stream{};

  interpreter::instructions::InstructionsWriter writer;
  interpreter::ast::VisitCode(code_stream, writer);
  const auto instructions_block = writer.MakeBlock();

  interpreter::instructions::ExecutionContext context{
      .input = input_stream, .output = output_stream, .variables = {}};
  instructions_block.Execute(context);

  return output_stream.str();
}

}  // namespace interpreter::test
