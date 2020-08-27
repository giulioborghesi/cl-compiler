#ifndef COOL_CODEGEN_CODEGEN_BASE_H
#define COOL_CODEGEN_CODEGEN_BASE_H

#include <cool/core/status.h>
#include <cool/ir/common.h>
#include <cool/ir/fwd.h>

#include <ostream>

namespace cool {

/// Forward declaration
class CodegenContext;

class CodegenBasePass {

public:
  CodegenBasePass() = default;
  virtual ~CodegenBasePass() = default;

  /// Program, class and attributes nodes
  virtual Status codegen(CodegenContext *context, AttributeNode *node,
                         std::ostream *ios) {
    return Status::Ok();
  }

  virtual Status codegen(CodegenContext *context, ClassNode *node,
                         std::ostream *ios);

  virtual Status codegen(CodegenContext *context, FormalNode *node,
                         std::ostream *ios) {
    return Status::Ok();
  }

  virtual Status codegen(CodegenContext *context, MethodNode *node,
                         std::ostream *ios);

  virtual Status codegen(CodegenContext *context, ProgramNode *node,
                         std::ostream *ios);

  /// Expressions nodes
  virtual Status codegen(CodegenContext *context, AssignmentExprNode *node,
                         std::ostream *ios);

  virtual Status codegen(CodegenContext *context,
                         BinaryExprNode<ArithmeticOpID> *node,
                         std::ostream *ios);

  virtual Status codegen(CodegenContext *context,
                         BinaryExprNode<ComparisonOpID> *node,
                         std::ostream *ios);

  virtual Status codegen(CodegenContext *context, BlockExprNode *node,
                         std::ostream *ios);

  virtual Status codegen(CodegenContext *context, BooleanExprNode *node,
                         std::ostream *ios) {
    return Status::Ok();
  }

  virtual Status codegen(CodegenContext *context, CaseBindingNode *node,
                         std::ostream *ios);

  virtual Status codegen(CodegenContext *context, CaseExprNode *node,
                         std::ostream *ios);

  virtual Status codegen(CodegenContext *context, DispatchExprNode *node,
                         std::ostream *ios);

  Status codegen(CodegenContext *context, ExprNode *node, std::ostream *ios) {
    return Status::Ok();
  }

  Status codegen(CodegenContext *context, IdExprNode *node, std::ostream *ios) {
    return Status::Ok();
  }

  virtual Status codegen(CodegenContext *context, IfExprNode *node,
                         std::ostream *ios);

  virtual Status codegen(CodegenContext *context, LetBindingNode *node,
                         std::ostream *ios);

  virtual Status codegen(CodegenContext *context, LetExprNode *node,
                         std::ostream *ios);

  virtual Status codegen(CodegenContext *context,
                         LiteralExprNode<int32_t> *node, std::ostream *ios) {
    return Status::Ok();
  }

  virtual Status codegen(CodegenContext *context,
                         LiteralExprNode<std::string> *node,
                         std::ostream *ios) {
    return Status::Ok();
  }

  virtual Status codegen(CodegenContext *context, NewExprNode *node,
                         std::ostream *ios) {
    return Status::Ok();
  }

  virtual Status codegen(CodegenContext *context, StaticDispatchExprNode *node,
                         std::ostream *ios);

  virtual Status codegen(CodegenContext *context, UnaryExprNode *node,
                         std::ostream *ios);

  virtual Status codegen(CodegenContext *context, WhileExprNode *node,
                         std::ostream *ios);
};

} // namespace cool

#endif
