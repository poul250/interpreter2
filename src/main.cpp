#include <iostream>
#include <visitor/visitor.h>

namespace visitor {

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

class DebugSourceVisitor: visitor::SourceVisitor {
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

int main() {
  std::cout << "Hello world";
  return 0;
}
