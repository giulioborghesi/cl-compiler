#ifndef COOL_CODEGEN_H
#define COOL_CODEGEN_H

#include <cool/core/status.h>
#include <cool/ir/common.h>
#include <cool/ir/fwd.h>

#include <iostream>

namespace cool {

/// Forward declaration
class Context;

class CodegenPass {

public:
  CodegenPass() = default;

  /// Program, class and attributes nodes
  Status codegen(Context *context, AttributeNode *node, std::iostream *ios);

  Status codegen(Context *context, ClassNode *node, std::iostream *ios);

  Status codegen(Context *context, FormalNode *node, std::iostream *ios) {
    return Status::Ok();
  }

  Status codegen(Context *context, MethodNode *node, std::iostream *ios);

  Status codegen(Context *context, ProgramNode *node, std::iostream *ios);

  /// Expressions nodes
  Status codegen(Context *context, AssignmentExprNode *node,
                 std::iostream *ios);

  Status codegen(Context *context, BinaryExprNode<ArithmeticOpID> *node,
                 std::iostream *ios);

  Status codegen(Context *context, BinaryExprNode<ComparisonOpID> *node,
                 std::iostream *ios);

  Status codegen(Context *context, BlockExprNode *node, std::iostream *ios);

  Status codegen(Context *context, BooleanExprNode *node, std::iostream *ios);

  Status codegen(Context *context, CaseBindingNode *node, std::iostream *ios);

  Status codegen(Context *context, CaseExprNode *node, std::iostream *ios);

  Status codegen(Context *context, DispatchExprNode *node, std::iostream *ios);

  Status codegen(Context *context, ExprNode *node, std::iostream *ios) {
    return Status::Ok();
  }

  Status codegen(Context *context, IdExprNode *node, std::iostream *ios) {
    return Status::Ok();
  }

  Status codegen(Context *context, IfExprNode *node, std::iostream *ios);

  Status codegen(Context *context, LetBindingNode *node, std::iostream *ios);

  Status codegen(Context *context, LetExprNode *node, std::iostream *ios);

  Status codegen(Context *context, LiteralExprNode<int32_t> *node,
                 std::iostream *ios);

  Status codegen(Context *context, LiteralExprNode<std::string> *node,
                 std::iostream *ios);

  Status codegen(Context *context, NewExprNode *node, std::iostream *ios);

  Status codegen(Context *context, StaticDispatchExprNode *node,
                 std::iostream *ios);

  Status codegen(Context *context, UnaryExprNode *node, std::iostream *ios);

  Status codegen(Context *context, WhileExprNode *node, std::iostream *ios);
};

} // namespace cool

#endif
