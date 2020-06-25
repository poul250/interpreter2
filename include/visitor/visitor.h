#pragma once
#include <optional>
#include <vector>
#include <utility>


namespace visitor {

class Type {
};

class Identifier {
};

class Constant {
};

class SourceVisitor {
public:
  virtual ~SourceVisitor() = 0;

  virtual void VisitProgram() = 0;

  virtual void VisitDescription(Type type,
                                std::vector<std::pair<Identifier, std::optional<Constant>>> variables) = 0;

  virtual void VisitRead(Identifier identifier) = 0;

  // TODO: replace Identifier with Expression
  virtual void VisitWrite(Identifier identifier) = 0;
};
} // namespace visitor
