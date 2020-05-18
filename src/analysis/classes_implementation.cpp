#include <cool/analysis/classes_implementation.h>
#include <cool/core/context.h>
#include <cool/core/logger_collection.h>
#include <cool/ir/class.h>

#include <unordered_set>

namespace cool {

namespace {

/// \brief Helper function that compares the return types of two methods
///
/// \note Two return types are considered equivalent if a. they are both
/// SELF_TYPE or b. they refer to the same class
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
  /// Fetch class registry, logger and symbol table
  auto *registry = context->classRegistry();
  auto *logger = context->logger();
  auto *symbolTable = context->symbolTable();

  /// Attribute id cannot be self
  if (node->id() == "self") {
    LOG_ERROR_MESSAGE_WITH_LOCATION(logger, node,
                                    "'self' is not a valid attribute name");
    return Status::Error();
  }

  /// Attribute must not have been defined before or in parent classes
  if (symbolTable->findKeyInTable(node->id())) {
    LOG_ERROR_MESSAGE_WITH_LOCATION(
        logger, node, "Attribute %s cannot be redefined", node->id().c_str());
    return Status::Error();
  }

  /// Attribute must be of valid type
  const auto typeName = node->typeName();
  if (typeName != "SELF_TYPE" && !registry->hasClass(typeName)) {
    LOG_ERROR_MESSAGE_WITH_LOCATION(logger, node,
                                    "Attribute %s has undefined type %s",
                                    node->id().c_str(), typeName.c_str());
    return Status::Error();
  }

  /// All good, insert symbol in table and return
  if (typeName == "SELF_TYPE") {
    auto type = registry->toSelfType(context->currentClassName());
    symbolTable->addElement(node->id(), type);
  } else {
    auto type = registry->toType(typeName);
    symbolTable->addElement(node->id(), type);
  }
  return Status::Ok();
}

Status ClassesImplementationPass::visit(Context *context, ClassNode *node) {
  /// Initialize class context, symbol table and method table
  context->setCurrentClassName(node->className());
  context->initializeTables();
  auto *symbolTable = context->symbolTable();

  /// Install self in symbol table
  auto type = context->classRegistry()->toSelfType(node->className());
  symbolTable->addElement("self", type);

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
  return classesImplementationOk ? Status::Ok() : Status::Error();
}

Status ClassesImplementationPass::visit(Context *context, MethodNode *node) {
  /// Fetch class registry, logger and method table
  auto *registry = context->classRegistry();
  auto *logger = context->logger();
  auto *methodTable = context->methodTable();

  /// Method cannot be defined multiple times
  bool methodImplementationOk = true;
  if (methodTable->findKeyInScope(node->id())) {
    LOG_ERROR_MESSAGE_WITH_LOCATION(
        logger, node, "Method %s cannot be redefined", node->id().c_str());
    return Status::Error();
  }

  /// Verify method formal parameters
  std::vector<ExprType> argsTypes;
  std::unordered_set<std::string> argsIds;
  for (auto argument : node->arguments()) {
    /// The type of a parameter cannot be SELF_TYPE
    if (argument->typeName() == "SELF_TYPE") {
      methodImplementationOk = false;
      LOG_ERROR_MESSAGE_WITH_LOCATION(
          logger, argument,
          "Type of parameter %s in method %s cannot be SELF_TYPE",
          argument->id().c_str(), node->id().c_str());
      continue;
    }

    /// The type of a parameter must be valid
    const auto &typeName = argument->typeName();
    if (!registry->hasClass(typeName)) {
      methodImplementationOk = false;
      LOG_ERROR_MESSAGE_WITH_LOCATION(
          logger, argument,
          "Type %s of parameter %s in method %s is not declared",
          typeName.c_str(), argument->id().c_str(), node->id().c_str());
      continue;
    }

    /// A parameter can only be used once in a method
    if (argsIds.count(argument->id())) {
      methodImplementationOk = false;
      LOG_ERROR_MESSAGE_WITH_LOCATION(
          logger, argument, "Parameter %s in method %s cannot be reused",
          argument->id().c_str(), node->id().c_str());
      continue;
    }

    /// self is not a valid parameter
    if (argument->id() == "self") {
      methodImplementationOk = false;
      LOG_ERROR_MESSAGE_WITH_LOCATION(
          logger, argument, "'self' in method %s is not a valid parameter name",
          node->id().c_str());
      continue;
    }

    /// No error detected, add argument to method parameters list
    argsTypes.push_back(registry->toType(typeName));
    argsIds.insert({argument->id()});
  }

  /// Return type must be defined or be SELF_TYPE
  ExprType returnType;
  const auto &returnTypeName = node->returnTypeName();
  if (returnTypeName == "SELF_TYPE") {
    const auto &typeName = context->currentClassName();
    returnType = registry->toSelfType(typeName);
  } else {
    if (!registry->hasClass(returnTypeName)) {
      LOG_ERROR_MESSAGE_WITH_LOCATION(
          logger, node, "Return type %s of method %s is not defined",
          returnTypeName.c_str(), node->id().c_str());
      methodImplementationOk = false;
    } else {
      returnType = registry->toType(returnTypeName);
    }
  }

  /// Stop if errors were detected
  if (!methodImplementationOk) {
    return Status::Error();
  }

  /// Methods defined in parent classes must have identical signatures
  const auto &id = node->id();
  if (methodTable->findKeyInTable(id)) {
    auto parentRecord = methodTable->get(id);

    /// Number of arguments must be the same
    if (parentRecord.argsCount() != argsTypes.size()) {
      LOG_ERROR_MESSAGE_WITH_LOCATION(
          logger, node,
          "Method %s overrides a parent class method, but the number of "
          "arguments is not the same. Expected %u arguments, found %lu",
          node->id().c_str(), parentRecord.argsCount(), argsTypes.size());
      return Status::Error();
    }

    /// Arguments types must be the same
    for (size_t i = 0; i < parentRecord.argsCount(); ++i) {
      if (argsTypes[i].typeID != parentRecord.argsTypes()[i].typeID) {
        methodImplementationOk = false;
        LOG_ERROR_MESSAGE_WITH_LOCATION(
            logger, node,
            "Type of argument %s in method %s differs from parent method. "
            "Expected %s, actual %s",
            node->arguments()[i]->id().c_str(), node->id().c_str(),
            registry->typeName(parentRecord.argsTypes()[i]).c_str(),
            registry->typeName(argsTypes[i]).c_str());
        return Status::Error();
      }
    }

    /// Return types must be the same
    if (!SameReturnType(returnType, parentRecord.returnType())) {
      methodImplementationOk = false;
      LOG_ERROR_MESSAGE_WITH_LOCATION(
          logger, node,
          "Return type of method %s differs from parent method. Expected %s, "
          "actual %s",
          node->id().c_str(),
          registry->typeName(parentRecord.returnType()).c_str(),
          registry->typeName(returnType).c_str());
    }
  }

  /// If an error was detected, return
  if (!methodImplementationOk) {
    return Status::Error();
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
    return GenericError("Error. Class arguments or methods contain errors");
  }
  return Status::Ok();
}

} // namespace cool
