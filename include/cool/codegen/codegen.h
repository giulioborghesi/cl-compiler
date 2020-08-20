#ifndef COOL_CODEGEN_H
#define COOL_CODEGEN_H

#include <cool/core/status.h>
#include <cool/ir/common.h>
#include <cool/ir/fwd.h>

namespace cool {

/// Forward declaration
class Context;

class CodegenPass {

public:
  CodegenPass() = default;

  /// Program, class and attributes nodes
  Status codegen(Context *context, AttributeNode *node) { return Status::Ok(); }

  Status codegen(Context *context, ClassNode *node) { return Status::Ok(); }

  Status codegen(Context *context, FormalNode *node) { return Status::Ok(); }

  Status codegen(Context *context, MethodNode *node);

  Status codegen(Context *context, ProgramNode *node) { return Status::Ok(); }

  /// Expressions nodes
  Status codegen(Context *context, AssignmentExprNode *node);

  Status codegen(Context *context, BinaryExprNode<ArithmeticOpID> *node);

  Status codegen(Context *context, BinaryExprNode<ComparisonOpID> *node);

  Status codegen(Context *context, BlockExprNode *node);

  Status codegen(Context *context, BooleanExprNode *node);

  Status codegen(Context *context, CaseBindingNode *node);

  Status codegen(Context *context, CaseExprNode *node);

  Status codegen(Context *context, DispatchExprNode *node);

  Status codegen(Context *context, ExprNode *node) { return Status::Ok(); }

  Status codegen(Context *context, IdExprNode *node) { return Status::Ok(); }

  Status codegen(Context *context, IfExprNode *node);

  Status codegen(Context *context, LetBindingNode *node);

  Status codegen(Context *context, LetExprNode *node);

  Status codegen(Context *context, LiteralExprNode<int32_t> *node);

  Status codegen(Context *context, LiteralExprNode<std::string> *node);

  Status codegen(Context *context, NewExprNode *node);

  Status codegen(Context *context, StaticDispatchExprNode *node);

  Status codegen(Context *context, UnaryExprNode *node);

  Status codegen(Context *context, WhileExprNode *node);
};

} // namespace cool

#endif
