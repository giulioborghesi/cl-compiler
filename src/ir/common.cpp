#include <cool/ir/common.h>

namespace cool {

bool operator==(const ExprType lhs, const ExprType rhs) {
  return lhs.typeID == rhs.typeID && lhs.isSelf == rhs.isSelf;
}

bool operator!=(const ExprType lhs, const ExprType rhs) {
  return !(lhs == rhs);
}

} // namespace cool
