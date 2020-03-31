#include <cool/analysis/type_check.h>
#include <cool/core/context.h>
#include <cool/ir/expr.h>

#include <iostream>

namespace cool {

Status TypeCheckPass::visit(Context *context, BinaryExprNode *node) {
  const auto *registry = context->classRegistry();
  const auto intTypeID = registry->typeID("Int");

  if (intTypeID == -1) {
    return GenericError("Error: Int type is not defined");
  }

  /// Type-check left subexpression
  auto statusLhs = node->lhs()->visitNode(context, this);
  if (!statusLhs.isOk()) {
    return statusLhs;
  }

  /// Type-check right subexpression
  auto statusRhs = node->rhs()->visitNode(context, this);
  if (!statusRhs.isOk()) {
    return statusRhs;
  }

  /// Type-check binary expression
  const auto lhsTypeID = node->lhs()->type().typeID;
  const auto rhsTypeID = node->rhs()->type().typeID;
  if (lhsTypeID != intTypeID || rhsTypeID != intTypeID) {
    return GenericError("Error: operand is not an integer");
  }

  /// Set expression type and return
  node->setType(ExprType{.typeID = intTypeID, .isSelf = false});
  return Status::Ok();
}

Status TypeCheckPass::visit(Context *context, BooleanExprNode *node) {
  const auto *registry = context->classRegistry();
  const auto boolTypeID = registry->typeID("Bool");

  node->setType(ExprType{.typeID = boolTypeID, .isSelf = false});
  return Status::Ok();
}

Status TypeCheckPass::visit(Context *context, IfExprNode *node) {
  const auto *registry = context->classRegistry();
  const auto boolTypeID = registry->typeID("Bool");

  if (boolTypeID == -1) {
    return GenericError("Error: Bool type is not defined");
  }

  /// Type-check if-expression
  auto statusIfExpr = node->ifExpr()->visitNode(context, this);
  if (!statusIfExpr.isOk()) {
    return statusIfExpr;
  }

  /// Type-check then-expression
  auto statusThenExpr = node->thenExpr()->visitNode(context, this);
  if (!statusThenExpr.isOk()) {
    return statusThenExpr;
  }

  /// Type-check else-expression
  auto statusElseExpr = node->elseExpr()->visitNode(context, this);
  if (!statusElseExpr.isOk()) {
    return statusElseExpr;
  }

  /// If-expression type must be Bool
  if (node->ifExpr()->type().typeID != boolTypeID) {
    return GenericError("Error: if-expression type is not Bool");
  }

  /// Compute expression type and return
  const auto thenType = node->thenExpr()->type();
  const auto elseType = node->elseExpr()->type();
  node->setType(registry->leastCommonAncestor(thenType, elseType));
  return Status::Ok();
}

Status TypeCheckPass::visit(Context *context, NewExprNode *node) {
  const auto *registry = context->classRegistry();

  /// SELF_TYPE needs a special treatment
  if (node->typeName() == "SELF_TYPE") {
    const auto typeID = registry->typeID(context->currentClassName());
    node->setType(ExprType{.typeID = typeID, .isSelf = true});
    return Status::Ok();
  }

  const auto typeID = registry->typeID(node->typeName());
  if (typeID == -1) {
    return GenericError("Error: undefined type in new expression");
  }
  node->setType(ExprType{.typeID = typeID, .isSelf = false});
  return Status::Ok();
}

} // namespace cool
