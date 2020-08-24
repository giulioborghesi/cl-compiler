#ifndef COOL_CODEGEN_CODEGEN_H
#define COOL_CODEGEN_CODEGEN_H

#include <cool/codegen/codegen_base.h>
#include <cool/core/status.h>
#include <cool/ir/common.h>
#include <cool/ir/fwd.h>

#include <iostream>

namespace cool {

/// Forward declaration
class CodegenContext;

class CodegenPass : public CodegenBasePass {

public:
  CodegenPass() = default;

  /// Program, class and attributes nodes
  Status codegen(CodegenContext *context, AttributeNode *node,
                 std::iostream *ios) final override;

  Status codegen(CodegenContext *context, ClassNode *node,
                 std::iostream *ios) final override;

  Status codegen(CodegenContext *context, MethodNode *node,
                 std::iostream *ios) final override;

  Status codegen(CodegenContext *context, ProgramNode *node,
                 std::iostream *ios) final override;

  /// Expressions nodes
  Status codegen(CodegenContext *context, AssignmentExprNode *node,
                 std::iostream *ios) final override;

  Status codegen(CodegenContext *context, BinaryExprNode<ArithmeticOpID> *node,
                 std::iostream *ios) final override;

  Status codegen(CodegenContext *context, BinaryExprNode<ComparisonOpID> *node,
                 std::iostream *ios) final override;

  Status codegen(CodegenContext *context, BlockExprNode *node,
                 std::iostream *ios) final override;

  Status codegen(CodegenContext *context, BooleanExprNode *node,
                 std::iostream *ios) final override;

  Status codegen(CodegenContext *context, CaseBindingNode *node,
                 std::iostream *ios) final override;

  Status codegen(CodegenContext *context, CaseExprNode *node,
                 std::iostream *ios) final override;

  Status codegen(CodegenContext *context, DispatchExprNode *node,
                 std::iostream *ios) final override;

  Status codegen(CodegenContext *context, IdExprNode *node, std::iostream *ios);

  Status codegen(CodegenContext *context, IfExprNode *node,
                 std::iostream *ios) final override;

  Status codegen(CodegenContext *context, LetBindingNode *node,
                 std::iostream *ios) final override;

  Status codegen(CodegenContext *context, LetExprNode *node,
                 std::iostream *ios) final override;

  Status codegen(CodegenContext *context, LiteralExprNode<int32_t> *node,
                 std::iostream *ios) final override;

  Status codegen(CodegenContext *context, LiteralExprNode<std::string> *node,
                 std::iostream *ios) final override;

  Status codegen(CodegenContext *context, NewExprNode *node,
                 std::iostream *ios) final override;

  Status codegen(CodegenContext *context, StaticDispatchExprNode *node,
                 std::iostream *ios) final override;

  Status codegen(CodegenContext *context, UnaryExprNode *node,
                 std::iostream *ios) final override;

  Status codegen(CodegenContext *context, WhileExprNode *node,
                 std::iostream *ios) final override;
};

} // namespace cool

#endif
