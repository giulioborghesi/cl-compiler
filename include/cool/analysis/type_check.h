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

  Status visit(Context *context,
               BinaryExprNode<ComparisonOpID> *node) final override;

  Status visit(Context *context, BlockExprNode *node) final override;

  Status visit(Context *context, BooleanExprNode *node) final override;

  Status visit(Context *context, CaseBindingNode *node) final override;

  Status visit(Context *context, CaseExprNode *node) final override;

  Status visit(Context *context, DispatchExprNode *node) final override;

  Status visit(Context *context, IdExprNode *node) final override;

  Status visit(Context *context, IfExprNode *node) final override;

  Status visit(Context *context, LetBindingNode *node) final override;

  Status visit(Context *context, LetExprNode *node) final override;

  Status visit(Context *context, LiteralExprNode<int32_t> *node) final override;

  Status visit(Context *context,
               LiteralExprNode<std::string> *node) final override;

  Status visit(Context *context, NewExprNode *node) final override;

  Status visit(Context *context, StaticDispatchExprNode *node) final override;

  Status visit(Context *context, UnaryExprNode *node) final override;

  Status visit(Context *context, WhileExprNode *node) final override;

private:
  /// Implement type-checking rule for binary expressions
  ///
  /// \param[in] context type-checking context
  /// \param[in] node binary expression node to type-check
  /// \param[in] returnType expression return type
  /// \param[in] func function implementing type-checking rule
  /// \return Stats::Ok() if type-check succeds, an error message otherwise
  template <typename OpType, typename FuncT>
  Status visitBinaryExpr(Context *context, BinaryExprNode<OpType> *node,
                         const ExprType &returnType, FuncT &&func);

  /// Implement type-checking rule for dispatch expressions
  ///
  /// \param[in] context type-checking context
  /// \param[in] node dispatch expression node to type-check
  /// \param[in] callerType caller type
  /// \param[in] returnType return type of dispatch expression
  template <typename DispatchExprT>
  Status visitDispatchExpr(Context *context, DispatchExprT *node,
                           const ExprType calleeType,
                           const ExprType returnType);

  /// Implement type-checking rule for IsVoid unary expression
  ///
  /// \param[in] context type-checking context
  /// \param[in] node unary expression node to type-check
  /// \return Status::Ok() if type-check succeds, an error message otherwise
  Status visitIsVoidExpr(Context *context, UnaryExprNode *node);

  /// Implement type-checking rule for boolean NOT and integer complement
  ///
  /// \param[in] context type-checking context
  /// \param[in] node unary expression node to type-check
  /// \param[in] expectedType expected type of unary expression operand
  /// \return Status::Ok() if type-check succeds, an error message otherwise
  Status visitNotOrCompExpr(Context *context, UnaryExprNode *node,
                            const std::string &expectedType);
};

} // namespace cool

#endif
