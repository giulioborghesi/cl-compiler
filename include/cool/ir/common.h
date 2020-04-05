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

/// Unary operations identifiers
enum class UnaryOpID : uint8_t { IsVoid, Not, Complement, Parenthesis };

/// Arithmetic operations identifiers
enum class ArithmeticOpID : uint8_t { Plus, Minus, Mult, Div };

/// Comparison operations identifiers
enum class ComparisonOpID : uint8_t { LessThan, LessThanOrEqual, Equal };

} // namespace cool

#endif
