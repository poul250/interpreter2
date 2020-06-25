#pragma once
#include <iosfwd>

namespace interpreter {

void interpret(std::istream &code, std::istream &stdin, std::ostream &stdout);

} // namespace interpreter
