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

  Status codegen(CodegenContext *context, ClassNode *node,
                 std::ostream *ios) final override;

  Status codegen(CodegenContext *context, LiteralExprNode<int32_t> *node,
                 std::ostream *ios) final override;

  Status codegen(CodegenContext *context, LiteralExprNode<std::string> *node,
                 std::ostream *ios) final override;

  Status codegen(CodegenContext *context, ProgramNode *node,
                 std::ostream *ios) final override;
};

} // namespace cool

#endif
