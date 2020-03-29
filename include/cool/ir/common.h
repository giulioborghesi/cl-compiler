#ifndef COOL_IR_COMMON_H
#define COOL_IR_COMMON_H

namespace cool {

/// Class representing an expression type
struct ExprT {
  unsigned typeId = -1;
  bool isSelf = false;
};

} // namespace cool

#endif
