#ifndef COOL_CODEGEN_CODEGEN_PREPARE_H
#define COOL_CODEGEN_CODEGEN_PREPARE_H

#include <cool/codegen/codegen_base.h>
#include <cool/core/status.h>
#include <cool/ir/common.h>
#include <cool/ir/fwd.h>

namespace cool {

/// Forward declaration
class CodegenContext;

class CodegenPreparePass : public CodegenBasePass {

public:
  CodegenPreparePass() = default;

  /// Program, class and attributes nodes
  Status codegen(CodegenContext *context, ClassNode *node,
                 std::ostream *ios) final override;

  Status codegen(CodegenContext *context, ProgramNode *node,
                 std::ostream *ios) final override;
};

} // namespace cool

#endif
