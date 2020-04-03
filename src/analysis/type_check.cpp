#include <cool/analysis/type_check.h>
#include <cool/core/context.h>
#include <cool/ir/expr.h>

#include <iostream>

namespace cool {

Status TypeCheckPass::visit(Context *context, AssignmentExprNode *node) {
  /// Type-check lhs of assignment expression
  auto statusId = node->id()->visitNode(context, this);
  if (!statusId.isOk()) {
    return statusId;
  }

  /// Type-check rhs of assignment expression
  auto statusValue = node->value()->visitNode(context, this);
  if (!statusValue.isOk()) {
    return statusValue;
  }

  /// Value type must be a subtype of id type
  const auto *registry = context->classRegistry();
  if (!registry->conformTo(node->value()->type(), node->id()->type())) {
    return GenericError("Error: value type is not a subtype of id type");
  }
  node->setType(node->value()->type());
  return Status::Ok();
}

Status TypeCheckPass::visit(Context *context,
                            BinaryExprNode<ArithmeticOpID> *node) {
  const auto *registry = context->classRegistry();
  const auto intTypeID = registry->typeID("Int");

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

Status TypeCheckPass::visit(Context *context, BlockExprNode *node) {
  for (auto &subNode : node->exprs()) {
    auto status = subNode->visitNode(context, this);
    if (!status.isOk()) {
      return status;
    }
  }

  /// Exprs must always contain at least one expression
  node->setType(node->exprs().back()->type());
  return Status::Ok();
}

Status TypeCheckPass::visit(Context *context, BooleanExprNode *node) {
  const auto *registry = context->classRegistry();
  const auto boolTypeID = registry->typeID("Bool");

  node->setType(ExprType{.typeID = boolTypeID, .isSelf = false});
  return Status::Ok();
}

Status TypeCheckPass::visit(Context *context, CaseNode *node) {
  auto *symbolTable = context->symbolTable();
  symbolTable->enterScope();

  /// Type-check expression after having modified locally the symbol table
  symbolTable->addElement(node->id()->idName(), node->idType());
  auto statusExpr = node->expr()->visitNode(context, this);

  symbolTable->exitScope();

  return statusExpr;
}

Status TypeCheckPass::visit(Context *context, CaseExprNode *node) {
  const auto &cases = node->cases();

  /// Process the first case and initialize the return type
  auto statusFirstExpr = cases[0]->visitNode(context, this);
  if (!statusFirstExpr.isOk()) {
    return statusFirstExpr;
  }
  ExprType exprType = cases[0]->expr()->type();

  /// Process the remaining cases
  const auto *registry = context->classRegistry();
  for (uint32_t i = 1; i < cases.size(); ++i) {
    auto statusCurrExpr = cases[i]->visitNode(context, this);
    if (!statusCurrExpr.isOk()) {
      return statusCurrExpr;
    }
    exprType =
        registry->leastCommonAncestor(exprType, cases[i]->expr()->type());
  }

  /// No error encountered, set type of expression and return
  node->setType(exprType);
  return Status::Ok();
}
/*
Status TypeCheckPass::visit(Context *context, DispatchExprNode *node) {
  /// Type-check expression if it exists
  if (node->hasExpr()) {
    auto statusExpr = node->expr()->visitNode(context, this);
    if (!statusExpr.isOk()) {
      return statusExpr;
    }
  }

  /// Get type ID of expression (self included) and complete type-check
  const auto typeID =
      node->hasExpr() ? node->expr()->type().typeID : context->currentClassID();
  return visitDispatchExpr(context, node, typeID);
}
*/

Status TypeCheckPass::visit(Context *context, IdExprNode *node) {
  auto *symbolTable = context->symbolTable();
  if (!symbolTable->findKeyInTable(node->idName())) {
    return GenericError("Error: identifier is undefined");
  }
  node->setType(symbolTable->get(node->idName()));
  return Status::Ok();
}

Status TypeCheckPass::visit(Context *context, IfExprNode *node) {
  const auto *registry = context->classRegistry();
  const auto boolTypeID = registry->typeID("Bool");

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

Status TypeCheckPass::visit(Context *context, LetBindingNode *node) {
  auto *registry = context->classRegistry();
  auto *symbolTable = context->symbolTable();

  if (node->hasExpr()) {
    /// Type-check rhs
    auto statusExpr = node->expr()->visitNode(context, this);
    if (!statusExpr.isOk()) {
      return statusExpr;
    }

    /// Type of rhs expression must be a subtype of formal id type
    if (!registry->conformTo(node->expr()->type(), node->idType())) {
      return GenericError("Error: expr type is not a subtype of id type");
    }
  }

  /// Type of subexpression can be left to default type
  return symbolTable->addElement(node->id()->idName(), node->idType());
}

Status TypeCheckPass::visit(Context *context, LetExprNode *node) {
  auto *symbolTable = context->symbolTable();

  auto unwindSymbolTable = [symbolTable](const uint32_t nCount) {
    for (uint32_t i = 0; i < nCount; ++i) {
      symbolTable->exitScope();
    }
    return;
  };

  /// Process bindings first
  uint32_t unwindCount = 0;
  for (auto &bindingNode : node->bindings()) {
    ++unwindCount;
    symbolTable->enterScope();
    auto statusBinding = bindingNode->visitNode(context, this);
    if (!statusBinding.isOk()) {
      unwindSymbolTable(unwindCount);
      return statusBinding;
    }
  }

  /// Type-check let body and unwind symbol table
  auto statusExpr = node->expr()->visitNode(context, this);
  unwindSymbolTable(unwindCount);

  /// Check for errors, assign type to let expression and return
  if (!statusExpr.isOk()) {
    return statusExpr;
  }
  node->setType(node->expr()->type());
  return Status::Ok();
}

Status TypeCheckPass::visit(Context *context, LiteralExprNode<int32_t> *node) {
  const auto *registry = context->classRegistry();
  const auto intTypeID = registry->typeID("Int");
  node->setType(ExprType{.typeID = intTypeID, .isSelf = false});
  return Status::Ok();
}

Status TypeCheckPass::visit(Context *context,
                            LiteralExprNode<std::string> *node) {
  const auto *registry = context->classRegistry();
  const auto stringTypeID = registry->typeID("String");
  node->setType(ExprType{.typeID = stringTypeID, .isSelf = false});
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

Status TypeCheckPass::visit(Context *context, UnaryExprNode *node) {
  /// Type-check subexpression
  auto statusExpr = node->expr()->visitNode(context, this);
  if (!statusExpr.isOk()) {
    return statusExpr;
  }

  /// Assign type to expression or return an error message on error
  switch (node->opID()) {
  case UnaryOpID::IsVoid: {
    return visitUnaryOpIsVoid(context, node);
  }
  case UnaryOpID::Not:
  case UnaryOpID::Complement: {
    const std::string type = node->opID() == UnaryOpID::Not ? "Bool" : "Int";
    return visitUnaryOpNotComp(context, node, type);
  }
  case UnaryOpID::Parenthesis: {
    node->setType(node->expr()->type());
    return Status::Ok();
  }
  default: {
    return GenericError("Error: unary operation not supported");
  }
  }

  /// Unreachable, added to suppress compiler warnings
  return Status::Ok();
}

Status TypeCheckPass::visit(Context *context, WhileExprNode *node) {
  /// Type-check loop condition expression
  auto statusLoopCond = node->loopCond()->visitNode(context, this);
  if (!statusLoopCond.isOk()) {
    return statusLoopCond;
  }

  const auto *registry = context->classRegistry();
  const auto boolTypeID = registry->typeID("Bool");
  if (node->loopCond()->type().typeID != boolTypeID) {
    return GenericError("Error: while-cond expression is not Bool");
  }

  /// Type-check loop body expression
  auto statusLoopBody = node->loopBody()->visitNode(context, this);
  if (!statusLoopBody.isOk()) {
    return statusLoopBody;
  }

  const auto objectTypeID = registry->typeID("Object");
  node->setType(ExprType{.typeID = objectTypeID, .isSelf = false});
  return Status::Ok();
}

Status TypeCheckPass::visitUnaryOpIsVoid(Context *context,
                                         UnaryExprNode *node) {
  /// Assign Bool type to expression
  const auto boolTypeID = context->classRegistry()->typeID("Bool");
  node->setType(ExprType{.typeID = boolTypeID, .isSelf = false});
  return Status::Ok();
}

Status TypeCheckPass::visitUnaryOpNotComp(Context *context, UnaryExprNode *node,
                                          const std::string &expectedType) {
  const auto expectedTypeID = context->classRegistry()->typeID(expectedType);

  /// Subexpression type must be bool
  if (node->expr()->type().typeID != expectedTypeID) {
    return GenericError("Error: operand of not is of incorrect type");
  }

  node->setType(ExprType{.typeID = expectedTypeID, .isSelf = false});
  return Status::Ok();
}
/*
template <typename DispatchExprT>
Status TypeCheckPass::visitDispatchExpr(Context *context, DispatchExprT *node,
                                        const ExprType exprType) {
  /// Method must have been defined
  const auto *methodTable = context->methodTable(typeID);
  if (!methodTable.findKeyInTable(node->funcName())) {
    return GenericError("Error: method not found");
  }

  /// Number of arguments must match
  const auto *method = methodTable->get(node->funcName());
  if (method->argSize() != node->argSize()) {
    return GenericError("Error: invalid number of arguments");
  }

  /// Type-check each argument
  const auto *registry = context->classRegistry();
  for (uint32_t i = 0; i < node->argSize(); ++i) {
    auto statusExpr = node->exprs()[i]->visitNode(context, this);
    if (!statusExpr.isOk()) {
      return statusExpr;
    }

    if (!registry->conformTo(node->exprs()[i]->type(),
                             method->exprs()[i]->type())) {
      return GenericError("Error: invalid argument type");
    }
  }

  /// Set expression type and return
  if (method->returnType().isSelf) {
    node->setType(exprType);
  } else {
    node->setType(method->returnType()) l
  }
  return Status::Ok();
}
*/

} // namespace cool
