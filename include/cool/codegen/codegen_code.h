#ifndef COOL_CODEGEN_CODEGEN_OBJECTS_INIT_H
#define COOL_CODEGEN_CODEGEN_OBJECTS_INIT_H

#include <cool/codegen/codegen_code_base.h>

namespace cool {

class CodegenObjectsInitPass : public CodegenCodePass {

public:
  CodegenObjectsInitPass() = default;
  ~CodegenObjectsInitPass() final override = default;

  Status codegen(CodegenContext *context, AttributeNode *node,
                 std::ostream *ios) final override;

  Status codegen(CodegenContext *context, ClassNode *node,
                 std::ostream *ios) final override;

  Status codegen(CodegenContext *context, ProgramNode *node,
                 std::ostream *ios) final override;
};

} // namespace cool

#endif
