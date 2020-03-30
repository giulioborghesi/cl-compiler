#ifndef COOL_IR_COMMON_H
#define COOL_IR_COMMON_H

#include <cstdlib>
#include <string>

namespace cool {

using IdentifierType = int32_t;

/// Class representing an expression type
struct ExprType {
  IdentifierType typeID = -1;
  bool isSelf = false;
};

} // namespace cool

#endif
