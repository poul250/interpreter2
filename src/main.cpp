#include <fstream>
#include <iostream>

#include "interpreter/instructions/writer.hpp"

// TODO: move it in library
void interpret(std::istream& code, std::istream& input, std::ostream& output) {
  interpreter::instructions::InstructionsWriter writer;
  interpreter::ast::VisitCode(code, writer);
  const auto& instructions = writer.GetInstructions();

  interpreter::instructions::ExecutionContext context{
      .input = input, .output = output, .variables = {}};
  for (const auto& instruction : instructions) {
    instruction->Execute(context);
  }
}

int main(int argc, char** argv) {
  if (argc == 1) {
    interpret(std::cin, std::cin, std::cout);
  } else {
    std::ifstream code{argv[1]};
    if (!code) {
      std::cout << "Error while opening file " << argv[1] << std::endl;
      return -1;
    }
    interpret(code, std::cin, std::cout);
  }
  return 0;
}
