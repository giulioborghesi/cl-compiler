#include <cool/analysis/classes_definition.h>
#include <cool/core/context.h>
#include <cool/core/logger_collection.h>
#include <cool/ir/class.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

namespace cool {

namespace {

/// \brief Implementation of cycles detector in class tree
///
/// \param[in] edges class tree edges
/// \param[in] root root node used by DFS algorithm
/// \param[in] visitedAll nodes visited so far
/// \param[in] visitedNow nodes visited in current search
/// \return Status::Ok() if no cycle is detected in the class inheritance tree
Status
ValidClassTreeImpl(const std::unordered_map<std::string, std::string> &edges,
                   const std::string &root,
                   std::unordered_set<std::string> &visitedAll,
                   std::unordered_set<std::string> &visitedNow) {
  /// Graph has a cycle, generate error message and exit
  if (visitedNow.count(root)) {
    return GenericError("Error: cyclic class dependency detected");
  }

  /// This branch has no cycle, exit early
  if (visitedAll.count(root)) {
    return Status::Ok();
  }

  visitedNow.insert(root);
  visitedAll.insert(root);

  /// Iterate on parent if applicable
  if (edges.count(root)) {
    const auto parent = edges.find(root)->second;
    auto status = ValidClassTreeImpl(edges, parent, visitedAll, visitedNow);
    if (!status.isOk()) {
      return status;
    }
  }

  /// Search succeeded
  visitedNow.erase(root);
  return Status::Ok();
}

/// \brief Helper function that looks for cycles in the class inheritance tree
///
/// \param[in] node pointer to program node in the AST
/// \return Status::Ok() if no cycle is detected in the class inheritance tree
Status ValidClassTree(ProgramNode *node) {
  std::unordered_map<std::string, std::string> edges;
  for (auto classNode : node->classes()) {
    if (classNode->hasParentClass()) {
      edges.insert({classNode->className(), classNode->parentClassName()});
    }
  }

  std::unordered_set<std::string> visitedNow;
  std::unordered_set<std::string> visitedAll;

  for (auto classNode : node->classes()) {
    const auto &root = classNode->className();
    auto status = ValidClassTreeImpl(edges, root, visitedAll, visitedNow);
    if (!status.isOk()) {
      return status;
    }
  }

  return Status::Ok();
}

} // namespace

Status ClassesDefinitionPass::visit(Context *context, ProgramNode *node) {
  bool classesDefinitionOk = true;
  auto *registry = context->classRegistry();
  auto *logger = context->logger();

  /// Built-in classes
  using StringSetType = std::unordered_set<std::string>;
  static StringSetType reservedClasses = {"Object", "IO", "Bool", "Int",
                                          "String"};

  /// Check first class definitions. A class must be defined once, cannot
  /// redefine a built-in class and its name cannot be SELF_TYPE
  std::unordered_map<std::string, ClassNodePtr> classNodes;
  for (auto classNode : node->classes()) {
    const auto &className = classNode->className();
    if (reservedClasses.count(className) && !classNode->builtIn()) {
      classesDefinitionOk = false;
      LOG_ERROR_MESSAGE_WITH_LOCATION(
          logger, classNode,
          "Class %s is a built-in class and cannot be redefined",
          className.c_str());
    } else if (classNodes.count(className)) {
      classesDefinitionOk = false;
      LOG_ERROR_MESSAGE_WITH_LOCATION(
          logger, classNode,
          "Class %s was defined at line %d and cannot be redefined",
          className.c_str(), classNode->lineLoc());
    } else if (className == "SELF_TYPE") {
      classesDefinitionOk = false;
      LOG_ERROR_MESSAGE_WITH_LOCATION(logger, classNode,
                                      "SELF_TYPE is not a valid class name");
    } else {
      classNodes.insert({className, classNode});
    }
  }

  if (!classesDefinitionOk) {
    return GenericError("Error: program contains incorrect class definitions");
  }

  /// Check now parent classes. Parent classes must be defined and cannot be
  /// one of Bool, Int or String
  StringSetType invalidParents = {"Bool", "Int", "String"};
  for (auto classNode : node->classes()) {
    if (!classNode->hasParentClass()) {
      continue;
    }

    const auto &parentClassName = classNode->parentClassName();
    if (!classNodes.count(parentClassName)) {
      classesDefinitionOk = false;
      LOG_ERROR_MESSAGE_WITH_LOCATION(logger, classNode,
                                      "Parent class %s is not defined",
                                      parentClassName.c_str());
    }

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

  /// Valid inheritance tree does not contain cycles
  auto statusCycles = ValidClassTree(node);
  if (!statusCycles.isOk()) {
    return statusCycles;
  }

  /// No error detected. Add classes to class registry and return
  for (auto classNode : node->classes()) {
    auto status = registry->addClass(classNode);
    if (!status.isOk()) {
      return status;
    }
  }

  /// All good, return ok
  return Status::Ok();
}

} // namespace cool