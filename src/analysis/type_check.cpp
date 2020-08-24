#include <cool/analysis/analysis_context.h>
#include <cool/analysis/type_check.h>
#include <cool/core/logger_collection.h>
#include <cool/ir/class.h>
#include <cool/ir/expr.h>

#include <iostream>
#include <unordered_set>

namespace cool {

Status TypeCheckPass::visit(AnalysisContext *context,
                            AssignmentExprNode *node) {
  auto *logger = context->logger();

  /// Variable must be present in symbol table
  const auto *symbolTable = context->symbolTable();
  if (!symbolTable->findKeyInTable(node->id())) {
    LOG_ERROR_MESSAGE_WITH_LOCATION(logger, node, "Variable %s is not defined",
                                    node->id().c_str());
    return Status::Error();
  }

  /// Variable cannot be self
  if (node->id() == "self") {
    LOG_ERROR_MESSAGE_WITH_LOCATION(logger, node, "Cannot assign to 'self'");
    return Status::Error();
  }

  /// Type-check right hand side of assignment expression
  auto statusValue = node->rhsExpr()->visitNode(context, this);
  if (!statusValue.isOk()) {
    return statusValue;
  }

  /// Get the type of the identifier
  const auto idType = symbolTable->get(node->id());

  /// Value type must be a subtype of id type
  const auto *registry = context->classRegistry();
  if (!registry->conformTo(node->rhsExpr()->type(), idType)) {
    LOG_ERROR_MESSAGE_WITH_LOCATION(
        logger, node,
        "Type of right hand side expression evaluates to %s, which is not a "
        "subtype of %s",
        registry->typeName(node->rhsExpr()->type()).c_str(),
        registry->typeName(idType).c_str());
    return Status::Error();
  }

  /// All good. Set node type and return
  node->setType(node->rhsExpr()->type());
  return Status::Ok();
}

Status TypeCheckPass::visit(AnalysisContext *context, AttributeNode *node) {
  /// Nothing to do if attribute has no initialization expression
  if (!node->initExpr()) {
    return Status::Ok();
  }

  /// Return early if an error occurred while evaluating the rhs
  if (!node->initExpr()->visitNode(context, this).isOk()) {
    return Status::Error();
  }

  /// Fetch symbol table and class registry
  auto registry = context->classRegistry();
  auto symbolTable = context->symbolTable();

  /// Type of initialization expression must conform to attribute type
  auto idType = symbolTable->get(node->id());
  if (!registry->conformTo(node->initExpr()->type(), idType)) {
    auto logger = context->logger();
    LOG_ERROR_MESSAGE_WITH_LOCATION(logger, node,
                                    "Type of init expression does not conform "
                                    "to type of attribute %s in class %s",
                                    node->id().c_str(),
                                    context->currentClassName().c_str());
    return Status::Error();
  }
  return Status::Ok();
}

Status TypeCheckPass::visit(AnalysisContext *context,
                            BinaryExprNode<ArithmeticOpID> *node) {
  const auto intTypeID = context->classRegistry()->typeID("Int");
  const ExprType returnType = ExprType{.typeID = intTypeID, .isSelf = false};

  auto typeCheckF = [context, node](const auto &lhsTypeID,
                                    const auto &rhsTypeID) {
    const auto intTypeID = context->classRegistry()->typeID("Int");
    if (lhsTypeID != intTypeID || rhsTypeID != intTypeID) {
      auto *logger = context->logger();
      LOG_ERROR_MESSAGE_WITH_LOCATION(
          logger, node,
          "Arithmetic expressions between non-integer types are not supported");
      return Status::Error();
    }
    return Status::Ok();
  };

  return visitBinaryExpr(context, node, returnType, typeCheckF);
}

Status TypeCheckPass::visit(AnalysisContext *context,
                            BinaryExprNode<ComparisonOpID> *node) {
  const ExprType returnType = context->classRegistry()->toType("Bool");

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
  auto typeCheckE = [context, node, types](const auto &lhsTypeID,
                                           const auto &rhsTypeID) {
    if (types.count(lhsTypeID) || types.count(rhsTypeID)) {
      if (lhsTypeID != rhsTypeID) {
        auto *registry = context->classRegistry();
        auto *logger = context->logger();
        LOG_ERROR_MESSAGE_WITH_LOCATION(
            logger, node,
            "Equality comparison only possible between objects of the same "
            "type for Int, String and Bool. Types of objects compared are %s "
            "and %s",
            registry->className(lhsTypeID).c_str(),
            registry->className(rhsTypeID).c_str());
        return Status::Error();
      }
    }
    return Status::Ok();
  };

  if (node->opID() == ComparisonOpID::Equal) {
    return visitBinaryExpr(context, node, returnType, typeCheckE);
  }
  return visitBinaryExpr(context, node, returnType, typeCheckC);
}

Status TypeCheckPass::visit(AnalysisContext *context, BlockExprNode *node) {
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

Status TypeCheckPass::visit(AnalysisContext *context, BooleanExprNode *node) {
  const auto *registry = context->classRegistry();
  node->setType(registry->toType("Bool"));
  return Status::Ok();
}

Status TypeCheckPass::visit(AnalysisContext *context, CaseBindingNode *node) {
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

Status TypeCheckPass::visit(AnalysisContext *context, CaseExprNode *node) {
  /// Fetch class registry
  const auto *registry = context->classRegistry();

  /// Typecheck expression first
  auto statusExpr = node->expr()->visitNode(context, this);
  if (!statusExpr.isOk()) {
    return statusExpr;
  }

  /// No duplicate case allowed
  std::unordered_set<ExprType> usedTypes;
  const auto &caseNodes = node->cases();
  for (auto caseNode : caseNodes) {
    auto statusCase = caseNode->visitNode(context, this);
    if (!statusCase.isOk()) {
      return statusCase;
    }

    if (usedTypes.count(caseNode->expr()->type())) {
      auto *logger = context->logger();
      LOG_ERROR_MESSAGE_WITH_LOCATION(
          logger, caseNode, "Types of case expressions must be unique");
      return Status::Error();
    }

    usedTypes.insert(caseNode->expr()->type());
  }

  /// Compute the return type
  ExprType exprType = caseNodes[0]->expr()->type();
  for (uint32_t i = 1; i < caseNodes.size(); ++i) {
    ExprType caseType = caseNodes[i]->expr()->type();
    exprType = registry->leastCommonAncestor(exprType, caseType);
  }

  /// Set type of expression and return
  node->setType(exprType);
  return Status::Ok();
}

Status TypeCheckPass::visit(AnalysisContext *context, ClassNode *node) {
  /// Set current class name and fetch symbol table
  context->setCurrentClassName(node->className());
  auto *symbolTable = context->symbolTable();

  /// Type-check each attribute
  bool isOk = true;
  for (auto attribute : node->attributes()) {
    if (!attribute->visitNode(context, this).isOk()) {
      isOk = false;
    }
  }

  /// Type-check each method
  for (auto method : node->methods()) {
    if (!method->visitNode(context, this).isOk()) {
      isOk = false;
    }
  }

  /// Return the status
  return isOk ? Status::Ok() : Status::Error();
}

Status TypeCheckPass::visit(AnalysisContext *context, DispatchExprNode *node) {
  /// Type-check expression if it exists
  if (node->hasExpr()) {
    auto statusExpr = node->expr()->visitNode(context, this);
    if (!statusExpr.isOk()) {
      return statusExpr;
    }
  }

  auto findCallerType = [context, node]() -> ExprType {
    const auto classID = context->currentClassID();
    if (node->hasExpr()) {
      return node->expr()->type();
    }
    return ExprType{.typeID = context->currentClassID(), .isSelf = true};
  };

  /// Determine type of calling expression and complete type-check
  const auto callerType = findCallerType();
  return visitDispatchExpr(context, node, callerType, callerType);
}

Status TypeCheckPass::visit(AnalysisContext *context, IdExprNode *node) {
  auto *symbolTable = context->symbolTable();
  auto *logger = context->logger();
  if (!symbolTable->findKeyInTable(node->id())) {
    LOG_ERROR_MESSAGE_WITH_LOCATION(logger, node, "Variable %s is not defined",
                                    node->id().c_str());
    return Status::Error();
  }
  node->setType(symbolTable->get(node->id()));
  return Status::Ok();
}

Status TypeCheckPass::visit(AnalysisContext *context, IfExprNode *node) {
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
  if (node->ifExpr()->type() != registry->toType("Bool")) {
    auto *logger = context->logger();
    LOG_ERROR_MESSAGE_WITH_LOCATION(
        logger, node->ifExpr(),
        "Condition in if construct must be of Bool type. Actual type: %s",
        registry->className(node->ifExpr()->type().typeID).c_str());
    return Status::Error();
  }

  /// Compute expression type and return
  const auto thenType = node->thenExpr()->type();
  const auto elseType = node->elseExpr()->type();
  node->setType(registry->leastCommonAncestor(thenType, elseType));
  return Status::Ok();
}

Status TypeCheckPass::visit(AnalysisContext *context, LetBindingNode *node) {
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

Status TypeCheckPass::visit(AnalysisContext *context, LetExprNode *node) {
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

Status TypeCheckPass::visit(AnalysisContext *context,
                            LiteralExprNode<int32_t> *node) {
  const auto *registry = context->classRegistry();
  node->setType(registry->toType("Int"));
  return Status::Ok();
}

Status TypeCheckPass::visit(AnalysisContext *context,
                            LiteralExprNode<std::string> *node) {
  const auto *registry = context->classRegistry();
  node->setType(registry->toType("String"));
  return Status::Ok();
}

Status TypeCheckPass::visit(AnalysisContext *context, MethodNode *node) {
  /// Fetch class registry
  auto registry = context->classRegistry();

  /// Fetch symbol table and enter new scope
  auto symbolTable = context->symbolTable();
  symbolTable->enterScope();

  /// Add method arguments to symbol table
  for (auto argument : node->arguments()) {
    const auto exprType = registry->toType(argument->typeName());
    symbolTable->addElement(argument->id(), exprType);
  }

  /// Type-check method body
  if (!node->body()->visitNode(context, this).isOk()) {
    return Status::Error();
  }

  /// Body type must be a subtype of return type
  const auto returnType =
      node->returnTypeName() == "SELF_TYPE"
          ? registry->toSelfType(context->currentClassName())
          : registry->toType(node->returnTypeName());
  if (!registry->conformTo(node->body()->type(), returnType)) {
    auto logger = context->logger();
    LOG_ERROR_MESSAGE_WITH_LOCATION(logger, node,
                                    "Type of body expression does not conform "
                                    "to return type of method %s in class %s",
                                    node->id().c_str(),
                                    context->currentClassName().c_str());
    return Status::Error();
  }

  /// All good, return ok
  return Status::Ok();
}

Status TypeCheckPass::visit(AnalysisContext *context, NewExprNode *node) {
  const auto *registry = context->classRegistry();

  /// SELF_TYPE needs a special treatment
  if (node->typeName() == "SELF_TYPE") {
    node->setType(registry->toSelfType(context->currentClassName()));
    return Status::Ok();
  }

  /// Type must be valid
  if (!registry->hasClass(node->typeName())) {
    auto *logger = context->logger();
    LOG_ERROR_MESSAGE_WITH_LOCATION(logger, node,
                                    "Type %s in new expression is not defined",
                                    node->typeName().c_str());
    return Status::Error();
  }

  /// Assign type to expression and return
  node->setType(registry->toType(node->typeName()));
  return Status::Ok();
}

Status TypeCheckPass::visit(AnalysisContext *context, ProgramNode *node) {
  /// Process all classes, regardless of whether errors are encountered or not
  bool isOk = true;
  for (auto classNode : node->classes()) {
    if (!classNode->visitNode(context, this).isOk()) {
      isOk = false;
    }
  }
  return isOk ? Status::Ok() : Status::Error();
}

Status TypeCheckPass::visit(AnalysisContext *context,
                            StaticDispatchExprNode *node) {
  auto *logger = context->logger();

  /// Type-check expression
  auto statusExpr = node->expr()->visitNode(context, this);
  if (!statusExpr.isOk()) {
    return statusExpr;
  }

  /// Dispatch type must exist
  const auto *registry = context->classRegistry();
  if (!registry->hasClass(node->callerClass())) {
    LOG_ERROR_MESSAGE_WITH_LOCATION(logger, node,
                                    "Dispatch type %s is not defined",
                                    node->callerClass().c_str());
    return Status::Error();
  }

  /// Compute caller and dispatch types
  const auto callerType = node->expr()->type();
  const auto dispatchType = registry->toType(node->callerClass());

  /// Caller type must conform to dispatch type
  if (!registry->conformTo(callerType, dispatchType)) {
    LOG_ERROR_MESSAGE_WITH_LOCATION(
        logger, node, "Caller type %s does not conform to dispatch type %s",
        registry->typeName(callerType).c_str(), node->callerClass().c_str());
    return Status::Error();
  }

  /// Finalize type-check
  const auto returnType = node->expr()->type();
  return visitDispatchExpr(context, node, dispatchType, callerType);
}

Status TypeCheckPass::visit(AnalysisContext *context, UnaryExprNode *node) {
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

Status TypeCheckPass::visit(AnalysisContext *context, WhileExprNode *node) {
  /// Type-check loop condition expression
  auto statusLoopCond = node->loopCond()->visitNode(context, this);
  if (!statusLoopCond.isOk()) {
    return statusLoopCond;
  }

  /// Type of loop condition must be bool
  const auto *registry = context->classRegistry();
  auto *logger = context->logger();
  if (node->loopCond()->type() != registry->toType("Bool")) {
    LOG_ERROR_MESSAGE_WITH_LOCATION(
        logger, node->loopCond(),
        "Loop condition must be of type Bool. Actual type: %s",
        registry->typeName(node->loopCond()->type()).c_str());
    return Status::Error();
  }

  /// Type-check loop body expression
  auto statusLoopBody = node->loopBody()->visitNode(context, this);
  if (!statusLoopBody.isOk()) {
    return statusLoopBody;
  }

  /// Type of while expression is Object
  node->setType(registry->toType("Object"));
  return Status::Ok();
}

template <typename OpType, typename FuncT>
Status TypeCheckPass::visitBinaryExpr(AnalysisContext *context,
                                      BinaryExprNode<OpType> *node,
                                      const ExprType &returnType,
                                      FuncT &&func) {
  const auto *registry = context->classRegistry();

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

Status TypeCheckPass::visitIsVoidExpr(AnalysisContext *context,
                                      UnaryExprNode *node) {
  /// Assign Bool type to isvoid expression and return
  node->setType(context->classRegistry()->toType("Bool"));
  return Status::Ok();
}

Status TypeCheckPass::visitNotOrCompExpr(AnalysisContext *context,
                                         UnaryExprNode *node,
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
Status TypeCheckPass::visitDispatchExpr(AnalysisContext *context,
                                        DispatchExprT *node,
                                        const ExprType dispatchType,
                                        const ExprType callerType) {
  auto *logger = context->logger();
  const auto *registry = context->classRegistry();

  /// Fetch method table
  const auto *methodTable = context->methodTable(dispatchType.typeID);
  if (!methodTable) {
    LOG_ERROR_MESSAGE_WITH_LOCATION(
        logger, node, "Method table for class %s has not been defined",
        registry->className(dispatchType.typeID).c_str());
    return Status::Error();
  }

  /// Search for method record in table
  if (!methodTable->findKeyInTable(node->methodName())) {
    LOG_ERROR_MESSAGE_WITH_LOCATION(
        logger, node, "Method %s of class %s has not been defined",
        node->methodName().c_str(),
        registry->className(dispatchType.typeID).c_str());
    return Status::Error();
  }

  /// Number of arguments and parameters must match
  const auto &methodRecord = methodTable->get(node->methodName());
  if (methodRecord.argsCount() != node->paramsCount()) {
    LOG_ERROR_MESSAGE_WITH_LOCATION(
        logger, node,
        "Method %s of class %s invoked with an invalid number of arguments. "
        "Expected: %d, actual: %d",
        node->methodName().c_str(),
        registry->className(dispatchType.typeID).c_str(),
        methodRecord.argsCount(), node->paramsCount());
    return Status::Error();
  }

  /// Type-check each parameter
  bool isOk = true;
  for (uint32_t i = 0; i < node->paramsCount(); ++i) {
    auto statusExpr = node->params()[i]->visitNode(context, this);
    if (!statusExpr.isOk()) {
      return statusExpr;
    }

    if (!registry->conformTo(node->params()[i]->type(),
                             methodRecord.argsTypes()[i])) {
      LOG_ERROR_MESSAGE_WITH_LOCATION(
          logger, node->params()[i],
          "Argument %d of method %s in class %s is of invalid type. Expected: "
          "%s, actual: %s",
          i + 1, node->methodName().c_str(),
          registry->className(dispatchType.typeID).c_str(),
          registry->className(methodRecord.argsTypes()[i].typeID).c_str(),
          registry->className(node->params()[i]->type().typeID).c_str());
      isOk = false;
    }
  }

  if (!isOk) {
    return Status::Error();
  }

  /// Set expression type and return
  const auto returnType = methodRecord.returnType();
  if (returnType.isSelf) {
    node->setType(callerType);
  } else {
    node->setType(returnType);
  }
  return Status::Ok();
}

template Status TypeCheckPass::visitDispatchExpr<DispatchExprNode>(
    AnalysisContext *context, DispatchExprNode *node,
    const ExprType dispatchType, const ExprType callerType);

} // namespace cool
