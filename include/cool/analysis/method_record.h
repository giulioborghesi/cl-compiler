#ifndef COOL_CORE_METHOD_H
#define COOL_CORE_METHOD_H

#include <cool/ir/common.h>

#include <cstdlib>
#include <vector>

namespace cool {

class MethodRecord {

public:
  MethodRecord(const ExprType &returnType, std::vector<ExprType> argsTypes)
      : returnType_(returnType), argsTypes_(std::move(argsTypes)) {}

  /// Return the number of method arguments
  ///
  /// \return the number of method arguments
  const uint32_t argsCount() const { return argsTypes_.size(); }

  /// Return the types of the method arguments
  ///
  /// \return a list of the types of the method arguments
  const std::vector<ExprType> &argsTypes() const { return argsTypes_; }

  /// Return the method return type
  ///
  /// \return the method return type
  const ExprType &returnType() const { return returnType_; }

private:
  const ExprType returnType_;
  const std::vector<ExprType> argsTypes_;
};

} // namespace cool

#endif
