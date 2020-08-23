#include <cool/analysis/analysis_context.h>
#include <cool/analysis/classes_definition.h>
#include <cool/core/logger_collection.h>
#include <cool/ir/class.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

namespace cool {

Status ClassesDefinitionPass::visit(AnalysisContext *context,
                                    ProgramNode *node) {
  bool classesDefinitionOk = true;
  auto *registry = context->classRegistry();
  auto *logger = context->logger();

  /// Built-in classes
  using StringSetType = std::unordered_set<std::string>;
  static StringSetType reservedClasses = {"Object", "IO", "Bool", "Int",
                                          "String"};

  /// Check class definitions
  for (auto classNode : node->classes()) {
    const auto &className = classNode->className();
    if (reservedClasses.count(className) && !classNode->builtIn()) {
      /// Classes cannot redefine built-in classes
      classesDefinitionOk = false;
      LOG_ERROR_MESSAGE_WITH_LOCATION(
          logger, classNode,
          "Class %s is a built-in class and cannot be redefined",
          className.c_str());
    } else if (registry->hasClass(className)) {
      /// Classes cannot be defined multiple times
      classesDefinitionOk = false;
      LOG_ERROR_MESSAGE_WITH_LOCATION(
          logger, classNode,
          "Class %s was defined at line %d and cannot be redefined",
          className.c_str(), classNode->lineLoc());
    } else if (className == "SELF_TYPE") {
      /// Class name cannot be SELF_TYPE
      classesDefinitionOk = false;
      LOG_ERROR_MESSAGE_WITH_LOCATION(logger, classNode,
                                      "SELF_TYPE is not a valid class name");
    } else {
      registry->addClass(classNode);
    }
  }

  if (!classesDefinitionOk) {
    return GenericError("Error: program contains incorrect class definitions");
  }

  /// Check parent classes
  StringSetType invalidParents = {"Bool", "Int", "String"};
  for (auto classNode : node->classes()) {
    if (!classNode->hasParentClass()) {
      continue;
    }

    /// Parent class must be defined
    const auto &parentClassName = classNode->parentClassName();
    if (!registry->hasClass(parentClassName)) {
      classesDefinitionOk = false;
      LOG_ERROR_MESSAGE_WITH_LOCATION(
          logger, classNode, "Parent class %s of class %s is not defined",
          parentClassName.c_str(), classNode->className().c_str());
    }

    /// Parent class cannot be Bool, Int or String
    if (invalidParents.count(parentClassName)) {
      classesDefinitionOk = false;
      LOG_ERROR_MESSAGE_WITH_LOCATION(
          logger, classNode, "Class %s cannot inherit from built-in class %s",
          classNode->className().c_str(), parentClassName.c_str());
    }
  }

  if (!classesDefinitionOk) {
    return GenericError("Error: parent classes either not defined or invalid");
  }

  /// Program must have a Main class
  if (!registry->hasClass("Main")) {
    return GenericError("Error: Main class is not defined");
  }

  /// Sort the classes and return
  return node->sortClasses();
}

} // namespace cool