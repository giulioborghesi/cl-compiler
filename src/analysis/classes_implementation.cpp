#include <cool/analysis/classes_implementation.h>
#include <cool/core/context.h>
#include <cool/ir/class.h>

namespace cool {

namespace {

/// \brief Helper function that compares the return types of two methods
///
/// \param[in] lhs first return type to compare
/// \param[in] rhs second return type to compare
/// \return True if the return types are the same, false otherwise
bool SameReturnType(const ExprType &lhs, const ExprType &rhs) {
  if (lhs.isSelf && rhs.isSelf) {
    return true;
  }

  if ((lhs.isSelf && !rhs.isSelf) || (!lhs.isSelf && rhs.isSelf)) {
    return false;
  }

  return lhs.typeID == rhs.typeID;
}

} // namespace

Status ClassesImplementationPass::visit(Context *context, AttributeNode *node) {
  /// Fetch class registry and symbol table
  auto registry = context->classRegistry();
  auto symbolTable = context->symbolTable();

  /// Attribute id cannot be self
  if (node->id() == "self") {
    return EmptyError();
  }

  /// Attribute must not have been defined before or in parent classes
  if (symbolTable->findKeyInTable(node->id())) {
    return EmptyError();
  }

  /// Attribute must be of valid type
  const auto typeName = node->typeName();
  if (typeName != "SELF_TYPE" && !registry->hasClass(typeName)) {
    return EmptyError();
  }

  /// All good, insert symbol in table and return
  if (typeName == "SELF_TYPE") {
    auto typeExpr = registry->typeExpression(context->currentClassName(), true);
    symbolTable->addElement(node->id(), typeExpr);
  } else {
    auto typeExpr = registry->typeExpression(typeName, false);
    symbolTable->addElement(node->id(), typeExpr);
  }
  return Status::Ok();
}

Status ClassesImplementationPass::visit(Context *context, ClassNode *node) {
  /// Initialize class context and symbol table
  context->setCurrentClassName(node->className());
  auto symbolTable = context->symbolTable();

  /// Install self in symbol table
  auto typeExpr =
      context->classRegistry()->typeExpression(node->className(), true);
  symbolTable->addElement("self", typeExpr);

  /// Formal types of class attributes must be valid
  bool classesImplementationOk = true;
  for (auto attributeNode : node->attributes()) {
    auto status = attributeNode->visitNode(context, this);
    if (!status.isOk()) {
      classesImplementationOk = false;
    }
  }

  /// Class methods must be properly defined
  for (auto methodNode : node->methods()) {
    auto status = methodNode->visitNode(context, this);
    if (!status.isOk()) {
      classesImplementationOk = false;
    }
  }

  if (!classesImplementationOk) {
    return EmptyError();
  }
  return Status::Ok();
}

Status ClassesImplementationPass::visit(Context *context, MethodNode *node) {
  /// Fetch class registry and method table
  auto registry = context->classRegistry();
  auto methodTable = context->methodTable();

  /// Method cannot be redefined in class
  bool methodImplementationOk = true;
  if (methodTable->findKeyInScope(node->id())) {
    methodImplementationOk = false;
  }

  /// Formal arguments types must be defined and not SELF_TYPE
  std::vector<ExprType> argsTypes;
  for (auto argument : node->arguments()) {
    if (argument->typeName() == "SELF_TYPE") {
      methodImplementationOk = false;
      continue;
    }

    const auto &typeName = argument->typeName();
    if (!registry->hasClass(typeName)) {
      methodImplementationOk = false;
    } else {
      argsTypes.push_back(registry->typeExpression(typeName, false));
    }
  }

  /// Return type must be defined or be SELF_TYPE
  ExprType returnType;
  const auto &returnTypeName = node->returnTypeName();
  if (returnTypeName == "SELF_TYPE") {
    const auto &typeName = context->currentClassName();
    returnType = registry->typeExpression(typeName, true);
  } else {
    if (!registry->hasClass(returnTypeName)) {
      methodImplementationOk = false;
    } else {
      returnType = registry->typeExpression(returnTypeName, false);
    }
  }

  if (!methodImplementationOk) {
    return EmptyError();
  }

  /// Methods defined in parent classes must have identical signatures
  const auto &id = node->id();
  if (methodTable->findKeyInTable(id)) {
    auto parentRecord = methodTable->get(id);

    /// Number of arguments must be the same
    if (parentRecord.argsCount() != argsTypes.size()) {
      return EmptyError();
    }

    /// Arguments types must be the same
    for (size_t i = 0; i < parentRecord.argsCount(); ++i) {
      if (argsTypes[i].typeID != parentRecord.argsTypes()[i].typeID) {
        return EmptyError();
      }
    }

    /// Return types must be the same
    if (!SameReturnType(returnType, parentRecord.returnType())) {
      return EmptyError();
    }
  }

  /// All good, add method signature to table and return
  methodTable->addElement(id, MethodRecord(returnType, std::move(argsTypes)));
  return Status::Ok();
}

Status ClassesImplementationPass::visit(Context *context, ProgramNode *node) {
  bool classesImplementationOk = true;
  for (auto classNode : node->classes()) {
    auto status = classNode->visitNode(context, this);
    if (!status.isOk()) {
      classesImplementationOk = false;
    }
  }

  if (!classesImplementationOk) {
    return EmptyError();
  }
  return Status::Ok();
}

} // namespace cool
