#ifndef COOL_CORE_METHOD_H
#define COOL_CORE_METHOD_H

#include <cool/ir/common.h>

#include <cstdlib>
#include <vector>

namespace cool {

class MethodRecord {

public:
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
  ExprType returnType() const { return returnType_; }

private:
  ExprType returnType_;
  std::vector<ExprType> argsTypes_;
};

} // namespace cool

#endif
