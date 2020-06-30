#include <iostream>
#include <fstream>
#include <lex/lexems.h>
#include <filesystem>
#include <syntax/visitor.h>

namespace interpreter::syntax {

std::ostream& operator<<(std::ostream& stream, const Type& type) {
  stream << "type";
  return stream;
}

std::ostream& operator<<(std::ostream& stream, const Constant& constant) {
  stream << "constant";
  return stream;
}

std::ostream& operator<<(std::ostream& stream, const Identifier& ident) {
  stream << "ident";
  return stream;
}

class DebugSourceVisitor: ModelVisitor {
  void VisitDescription(Type type,
                        std::vector<std::pair<Identifier, std::optional<Constant>>> variables) {
    std::cout << "visit description with type" << type
        << "and variables:\n";
    for (const auto& [ident, val] : variables) {
      std::cout << ident << " " << (val.value_or(Constant{})) << std::endl;
    }

  }

  void VisitRead(Identifier identifier) {
    std::cout << "read " << identifier;
  }

  // TODO: replace Identifier with Expression
  void VisitWrite(Identifier identifier) {
    std::cout << "write " << identifier;
  }
};

}

void test_lexer(std::istream& input) {
  namespace lexems = interpreter::lexems;
  lexems::Lexer lexer{input};

  try {
    for (;;) {
      const auto& lex = lexer.GetNext();
      std::cout << lex << std::endl;
      if (lex.type == lexems::Type::NONE) {
        break;
      }
    }
  } catch (const std::runtime_error& exc) {
    std::cout << exc.what();
  } catch (const interpreter::lexems::LexicalError& ecx) {
    std::cout << ecx.what();
  }
}

void process(std::istream& input) {
  test_lexer(input);
}

int main(int argc, char** argv) {
  if (argc == 1) {
    process(std::cin);
  } else {
    std::ifstream input{argv[1]};
    if (!input) {
      std::cout << "Error while opening file " << argv[1] << std::endl;
      return -1;
    }
    process(input);
  }
  return 0;
}
