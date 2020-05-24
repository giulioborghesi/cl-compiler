#ifndef COOL_IR_COMMON_H
#define COOL_IR_COMMON_H

#include <cstdlib>
#include <string>

namespace cool {

using IdentifierType = int32_t;

/// Class representing an expression type
struct ExprType {
  IdentifierType typeID = 0;
  bool isSelf = false;
};

/// Equality operator between ExprType objects
///
/// \param[in] lhs first object to compare
/// \param[in] rhs second object to compare
/// \return true if the two expression types are the same, false otherwise
bool operator==(const ExprType lhs, const ExprType rhs);

/// Inequality operator between ExprType objects
///
/// \param[in] lhs first object to compare
/// \param[in] rhs second object to compare
/// \return true if the two expression types are not the same, false otherwise
bool operator!=(const ExprType lhs, const ExprType rhs);

/// Arithmetic operations identifiers
enum class ArithmeticOpID : uint8_t { Plus, Minus, Mult, Div };

/// Comparison operations identifiers
enum class ComparisonOpID : uint8_t { LessThan, LessThanOrEqual, Equal };

/// Unary operations identifiers
enum class UnaryOpID : uint8_t { IsVoid, Not, Complement };

} // namespace cool

#endif
