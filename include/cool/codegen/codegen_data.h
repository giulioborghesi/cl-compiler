#ifndef COOL_CODEGEN_CODEGEN_DATA_H
#define COOL_CODEGEN_CODEGEN_DATA_H

#include <cool/codegen/codegen_base.h>
#include <cool/core/status.h>
#include <cool/ir/common.h>
#include <cool/ir/fwd.h>

namespace cool {

/// Forward declaration
class CodegenContext;

class CodegenDataPass : public CodegenBasePass {

public:
  CodegenDataPass() = default;

  /// Program, class and attributes nodes
  Status codegen(CodegenContext *context, ClassNode *node,
                 std::ostream *ios) final override;

  Status codegen(CodegenContext *context, ProgramNode *node,
                 std::ostream *ios) final override;
};

} // namespace cool

#endif
