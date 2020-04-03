#ifndef COOL_ANALYSIS_TYPE_CHECK_H
#define COOL_ANALYSIS_TYPE_CHECK_H

#include <cool/analysis/pass.h>
#include <cool/ir/fwd.h>

#include <cstdlib>
#include <string>

namespace cool {

/// Forward declarations
class Context;

/// Class that implements a type-check pass over the abstract syntax tree. This
/// pass will infer and type-check the type of each expression in the input
/// program
class TypeCheckPass : public Pass {

public:
  TypeCheckPass() = default;
  ~TypeCheckPass() final override = default;

  Status visit(Context *context, AssignmentExprNode *node) final override;

  Status visit(Context *context,
               BinaryExprNode<ArithmeticOpID> *node) final override;

  Status visit(Context *context, BlockExprNode *node) final override;

  Status visit(Context *context, BooleanExprNode *node) final override;

  Status visit(Context *context, CaseNode *node) final override;

  Status visit(Context *context, CaseExprNode *node) final override;

  // Status visit(Context *context, DispatchExprNode *node) final override;

  Status visit(Context *context, IdExprNode *node) final override;

  Status visit(Context *context, IfExprNode *node) final override;

  Status visit(Context *context, LetBindingNode *node) final override;

  Status visit(Context *context, LetExprNode *node) final override;

  Status visit(Context *context, LiteralExprNode<int32_t> *node) final override;

  Status visit(Context *context,
               LiteralExprNode<std::string> *node) final override;

  Status visit(Context *context, NewExprNode *node) final override;

  Status visit(Context *context, UnaryExprNode *node) final override;

  Status visit(Context *context, WhileExprNode *node) final override;

private:
  Status visitUnaryOpIsVoid(Context *context, UnaryExprNode *node);

  Status visitUnaryOpNotComp(Context *context, UnaryExprNode *node,
                             const std::string &expectedType);
};

} // namespace cool

#endif
