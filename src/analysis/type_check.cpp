#include <cool/analysis/type_check.h>
#include <cool/core/context.h>
#include <cool/ir/expr.h>

#include <iostream>
#include <unordered_set>

namespace cool {

Status TypeCheckPass::visit(Context *context, AssignmentExprNode *node) {
  /// Variable must be present in symbol table
  const auto *symbolTable = context->symbolTable();
  if (!symbolTable->findKeyInTable(node->id())) {
    return GenericError("Error: undefined variable");
  }
  const auto idType = symbolTable->get(node->id());

  /// Type-check right hand side of assignment expression
  auto statusValue = node->rhsExpr()->visitNode(context, this);
  if (!statusValue.isOk()) {
    return statusValue;
  }

  /// Value type must be a subtype of id type
  const auto *registry = context->classRegistry();
  if (!registry->conformTo(node->rhsExpr()->type(), idType)) {
    return GenericError("Error: value type is not a subtype of id type");
  }
  node->setType(node->rhsExpr()->type());
  return Status::Ok();
}

Status TypeCheckPass::visit(Context *context,
                            BinaryExprNode<ArithmeticOpID> *node) {
  const auto intTypeID = context->classRegistry()->typeID("Int");
  const ExprType returnType = ExprType{.typeID = intTypeID, .isSelf = false};

  auto typeCheckF = [context](const auto &lhsTypeID, const auto &rhsTypeID) {
    const auto intTypeID = context->classRegistry()->typeID("Int");
    if (lhsTypeID != intTypeID || rhsTypeID != intTypeID) {
      return GenericError(
          "Error: only integer operands allowed in arithmetic expressions");
    }
    return Status::Ok();
  };

  return visitBinaryExpr(context, node, returnType, typeCheckF);
}

Status TypeCheckPass::visit(Context *context,
                            BinaryExprNode<ComparisonOpID> *node) {
  const auto boolTypeID = context->classRegistry()->typeID("Bool");
  const ExprType returnType = ExprType{.typeID = boolTypeID, .isSelf = false};

  /// Allowed types in equality expression
  std::unordered_set<IdentifierType> types;
  types.insert(context->classRegistry()->typeID("Bool"));
  types.insert(context->classRegistry()->typeID("Int"));
  types.insert(context->classRegistry()->typeID("String"));

  /// Type-check function for comparison expressions
  auto typeCheckC = [context](const auto &lhsTypeID, const auto &rhsTypeID) {
    const auto intTypeID = context->classRegistry()->typeID("Int");
    if (lhsTypeID != intTypeID || rhsTypeID != intTypeID) {
      return GenericError(
          "Error: only integer operands allowed in comparison expressions");
    }
    return Status::Ok();
  };

  /// Type-check function for equality expressions
  auto typeCheckE = [context, types](const auto &lhsTypeID,
                                     const auto &rhsTypeID) {
    if (types.count(lhsTypeID) || types.count(rhsTypeID)) {
      if (lhsTypeID != rhsTypeID) {
        return GenericError("Error: incompatible types");
      }
    }
    return Status::Ok();
  };

  if (node->opID() == ComparisonOpID::Equal) {
    return visitBinaryExpr(context, node, returnType, typeCheckE);
  }
  return visitBinaryExpr(context, node, returnType, typeCheckC);
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

Status TypeCheckPass::visit(Context *context, CaseBindingNode *node) {
  const auto *registry = context->classRegistry();
  auto *symbolTable = context->symbolTable();
  symbolTable->enterScope();

  /// Type must be valid and not SELF_TYPE
  const auto &typeName = node->typeName();
  if (typeName == "SELF_TYPE" || !registry->hasClass(typeName)) {
    return GenericError("Error: invalid type of case binding");
  }

  /// Modify symbol table
  const auto typeID = registry->typeID(node->typeName());
  const auto exprType = ExprType{.typeID = typeID, .isSelf = false};
  symbolTable->addElement(node->id(), exprType);

  /// Type-check case expression, exit scope and return
  auto statusExpr = node->expr()->visitNode(context, this);
  symbolTable->exitScope();
  return statusExpr;
}

Status TypeCheckPass::visit(Context *context, CaseExprNode *node) {
  const auto &cases = node->cases();

  /// Process the first case and initialize the return type
  auto statusFirstCase = cases[0]->visitNode(context, this);
  if (!statusFirstCase.isOk()) {
    return statusFirstCase;
  }
  ExprType exprType = cases[0]->expr()->type();

  /// Process the remaining cases
  const auto *registry = context->classRegistry();
  for (uint32_t i = 1; i < cases.size(); ++i) {
    auto statusCurrCase = cases[i]->visitNode(context, this);
    if (!statusCurrCase.isOk()) {
      return statusCurrCase;
    }
    exprType =
        registry->leastCommonAncestor(exprType, cases[i]->expr()->type());
  }

  /// No error encountered, set type of expression and return
  node->setType(exprType);
  return Status::Ok();
}

Status TypeCheckPass::visit(Context *context, DispatchExprNode *node) {
  /// Type-check expression if it exists
  if (node->hasExpr()) {
    auto statusExpr = node->expr()->visitNode(context, this);
    if (!statusExpr.isOk()) {
      return statusExpr;
    }
  }

  auto findCallerType = [context, node]() -> ExprType {
    const auto classID = context->currentClassID();
    if (!node->hasExpr() || node->expr()->type().typeID == classID) {
      return ExprType{.typeID = classID, .isSelf = false};
    }
    return node->expr()->type();
  };

  auto findReturnType = [context, node]() -> ExprType {
    if (node->hasExpr()) {
      return node->expr()->type();
    }
    return ExprType{.typeID = context->currentClassID(), .isSelf = true};
  };

  /// Determine type of calling expression and complete type-check
  const auto callerType = findCallerType();
  const auto returnType = findReturnType();
  return visitDispatchExpr(context, node, callerType, returnType);
}

Status TypeCheckPass::visit(Context *context, IdExprNode *node) {
  auto *symbolTable = context->symbolTable();
  if (!symbolTable->findKeyInTable(node->id())) {
    return GenericError("Error: undefined variable");
  }
  node->setType(symbolTable->get(node->id()));
  return Status::Ok();
}

Status TypeCheckPass::visit(Context *context, IfExprNode *node) {
  const auto *registry = context->classRegistry();

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
  const auto boolTypeID = registry->typeID("Bool");
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

  /// Helper function to determine the let binding type
  auto getBindingType = [context, registry, node]() {
    if (node->typeName() == "SELF_TYPE") {
      return ExprType{.typeID = context->currentClassID(), .isSelf = true};
    }
    const auto typeID = registry->typeID(node->typeName());
    return ExprType{.typeID = typeID, .isSelf = false};
  };

  /// Type must be valid or SELF_TYPE
  const auto &typeName = node->typeName();
  if (!registry->hasClass(typeName) && typeName != "SELF_TYPE") {
    return GenericError("Error: invalid type of let binding");
  }

  /// Type-check let binding initialization expression if needed
  const auto bindingType = getBindingType();
  if (node->hasExpr()) {
    auto statusExpr = node->expr()->visitNode(context, this);
    if (!statusExpr.isOk()) {
      return statusExpr;
    }

    /// Type of rhs expression must be a subtype of formal id type
    if (!registry->conformTo(node->expr()->type(), bindingType)) {
      return GenericError(
          "Error: expression type is not a subtype of let binding type");
    }
  }
  return symbolTable->addElement(node->id(), bindingType);
}

Status TypeCheckPass::visit(Context *context, LetExprNode *node) {
  auto *symbolTable = context->symbolTable();

  /// Helper function to unwind the symbol table
  auto unwindSymbolTable = [symbolTable](const uint32_t nCount) {
    for (uint32_t iCount = 0; iCount < nCount; ++iCount) {
      symbolTable->exitScope();
    }
    return;
  };

  /// Process let bindings
  uint32_t unwindCount = 0;
  for (auto bindingNode : node->bindings()) {
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
    const auto typeID = context->currentClassID();
    node->setType(ExprType{.typeID = typeID, .isSelf = true});
    return Status::Ok();
  }

  /// Type must be valid
  if (!registry->hasClass(node->typeName())) {
    return GenericError("Error: undefined type in new expression");
  }

  /// Assign type to expression and return
  const auto typeID = registry->typeID(node->typeName());
  node->setType(ExprType{.typeID = typeID, .isSelf = false});
  return Status::Ok();
}

Status TypeCheckPass::visit(Context *context, StaticDispatchExprNode *node) {
  /// Type-check expression
  auto statusExpr = node->expr()->visitNode(context, this);
  if (!statusExpr.isOk()) {
    return statusExpr;
  }

  /// Dispatch type must exist
  const auto *registry = context->classRegistry();
  if (!registry->hasClass(node->callerClass())) {
    return GenericError("Error: static dispatch type is not defined");
  }

  /// Compute caller type
  const auto callerTypeID = registry->typeID(node->callerClass());
  const auto callerType = ExprType{.typeID = callerTypeID, .isSelf = false};

  /// Compute return type and complete type-check
  const auto returnType = node->expr()->type();
  return visitDispatchExpr(context, node, callerType, returnType);
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
    return visitIsVoidExpr(context, node);
  }
  case UnaryOpID::Not:
  case UnaryOpID::Complement: {
    const std::string type = node->opID() == UnaryOpID::Not ? "Bool" : "Int";
    return visitNotOrCompExpr(context, node, type);
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

  /// Type of loop condition must be bool
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

  /// Type of while expression is Object
  const auto objectTypeID = registry->typeID("Object");
  node->setType(ExprType{.typeID = objectTypeID, .isSelf = false});
  return Status::Ok();
}

template <typename OpType, typename FuncT>
Status
TypeCheckPass::visitBinaryExpr(Context *context, BinaryExprNode<OpType> *node,
                               const ExprType &returnType, FuncT &&func) {
  const auto *registry = context->classRegistry();
  const auto intTypeID = registry->typeID("Int");

  /// Type-check left subexpression
  auto statusLhs = node->lhsExpr()->visitNode(context, this);
  if (!statusLhs.isOk()) {
    return statusLhs;
  }

  /// Type-check right subexpression
  auto statusRhs = node->rhsExpr()->visitNode(context, this);
  if (!statusRhs.isOk()) {
    return statusRhs;
  }

  /// Type-check binary expression
  const auto lhsTypeID = node->lhsExpr()->type().typeID;
  const auto rhsTypeID = node->rhsExpr()->type().typeID;
  auto status = func(lhsTypeID, rhsTypeID);
  if (!status.isOk()) {
    return status;
  }

  /// Set expression type and return
  node->setType(returnType);
  return Status::Ok();
}

Status TypeCheckPass::visitIsVoidExpr(Context *context, UnaryExprNode *node) {
  /// Assign Bool type to isvoid expression
  const auto boolTypeID = context->classRegistry()->typeID("Bool");
  node->setType(ExprType{.typeID = boolTypeID, .isSelf = false});
  return Status::Ok();
}

Status TypeCheckPass::visitNotOrCompExpr(Context *context, UnaryExprNode *node,
                                         const std::string &expectedType) {
  const auto expectedTypeID = context->classRegistry()->typeID(expectedType);

  /// Subexpression type must be bool for not, int for complement
  if (node->expr()->type().typeID != expectedTypeID) {
    return GenericError(
        "Error: operand of unary expression is of incorrect type");
  }

  node->setType(node->expr()->type());
  return Status::Ok();
}

template <typename DispatchExprT>
Status TypeCheckPass::visitDispatchExpr(Context *context, DispatchExprT *node,
                                        const ExprType callerType,
                                        const ExprType returnType) {
  /// Fetch method table
  const auto *methodTable = context->methodTable(callerType.typeID);
  if (!methodTable) {
    return GenericError("Error: type not defined");
  }

  /// Search for method record in table
  if (!methodTable->findKeyInTable(node->methodName())) {
    return GenericError("Error: method not found");
  }

  /// Number of arguments and parameters must match
  const auto &methodRecord = methodTable->get(node->methodName());
  if (methodRecord.argsCount() != node->paramsCount()) {
    return GenericError("Error: invalid number of arguments");
  }

  /// Type-check each parameter
  const auto *registry = context->classRegistry();
  for (uint32_t i = 0; i < node->paramsCount(); ++i) {
    auto statusExpr = node->params()[i]->visitNode(context, this);
    if (!statusExpr.isOk()) {
      return statusExpr;
    }

    if (!registry->conformTo(node->params()[i]->type(),
                             methodRecord.argsTypes()[i])) {
      return GenericError("Error: invalid argument type");
    }
  }

  /// Set expression type and return
  const auto formalReturnType = methodRecord.returnType();
  if (formalReturnType.isSelf) {
    node->setType(returnType);
  } else {
    node->setType(formalReturnType);
  }
  return Status::Ok();
}

template Status TypeCheckPass::visitDispatchExpr<DispatchExprNode>(
    Context *context, DispatchExprNode *node, const ExprType dispatchType,
    const ExprType returnType);

} // namespace cool
