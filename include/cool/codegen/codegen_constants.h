#ifndef COOL_CODEGEN_CODEGEN_CONSTANTS_H
#define COOL_CODEGEN_CODEGEN_CONSTANTS_H

#include <cool/codegen/codegen_base.h>
#include <cool/ir/fwd.h>

namespace cool {

/// Forward declaration
class CodegenContext;

class CodegenConstantsPass : public CodegenBasePass {

public:
  CodegenConstantsPass() = default;
  ~CodegenConstantsPass() final override = default;

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
