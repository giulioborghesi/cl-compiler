#ifndef COOL_CODEGEN_CODEGEN_CONSTANT_H
#define COOL_CODEGEN_CODEGEN_CONSTANT_H

#include <cool/codegen/codegen_base.h>
#include <cool/ir/fwd.h>

namespace cool {

/// Forward declaration
class CodegenContext;

class CodegenConstantPass : public CodegenBasePass {

public:
  CodegenConstantPass() = default;
  ~CodegenConstantPass() final override = default;

  Status codegen(CodegenContext *context, LiteralExprNode<std::string> *node,
                 std::ostream *ios) final override;
};

} // namespace cool

#endif
