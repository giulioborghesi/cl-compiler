#include <cool/analysis/classes_definition.h>
#include <cool/core/context.h>
#include <cool/ir/class.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

namespace cool {

namespace {

/// Detect cycles in class tree
///
/// \param[in] tree class inheritance tree
/// \param[in] parent parent node for DFS algorithm
/// \param[inout] visitedNow set of nodes visited in current DFS iteration
/// \param[inout] visitedAll set of visited nodes
/// \param[out] sortedClasses list of classes sorted in topological order
/// \return Status::Ok() on success
Status detectCyclesInClassTree(
    const std::unordered_map<IdentifierType, std::vector<IdentifierType>> &tree,
    const IdentifierType &parent,
    std::unordered_set<IdentifierType> *visitedNow,
    std::unordered_set<IdentifierType> *visitedAll,
    std::vector<IdentifierType> *sortedClasses) {
  /// Check for cyclic dependency
  if (visitedNow->count(parent)) {
    return GenericError("Error: cyclic class dependency detected");
  }

  /// Do nothing if node was processed already
  if (visitedAll->count(parent)) {
    return Status::Ok();
  }

  /// Add node to visited lists
  visitedNow->insert(parent);
  visitedAll->insert(parent);

  /// Recurse on children nodes
  auto kvIterator = tree.find(parent);
  if (kvIterator != tree.end()) {
    for (auto child : kvIterator->second) {
      auto status = detectCyclesInClassTree(tree, child, visitedNow, visitedAll,
                                            sortedClasses);
      if (!status.isOk()) {
        return status;
      }
    }
  }

  /// Remove node from visited list, add class to list and return
  visitedNow->erase(visitedNow->find(parent));
  sortedClasses->push_back(parent);
  return Status::Ok();
}

} // namespace

Status ClassesDefinitionPass::visit(Context *context, AttributeNode *node) {
  const auto *classRegistry = context->classRegistry();

  /// Attributes cannot be redefined
  auto *symbolTable = context->symbolTable();
  if (symbolTable->findKeyInTable(node->id())) {
    return GenericError("Error: cannot override attribute");
  }

  /// If attribute is self type, add it to symbol table and return
  const auto classID = context->currentClassID();
  const ExprType classType{.typeID = classID, .isSelf = true};
  if (node->typeName() == "SELF_TYPE") {
    symbolTable->addElement(node->id(), classType);
  }

  /// Attribute type cannot be a subtype of current class
  const auto typeID = classRegistry->typeID(node->typeName());
  const ExprType idType{.typeID = typeID, .isSelf = false};
  if (typeID != classID && classRegistry->conformTo(idType, classType)) {
    return GenericError("Error: attribute type is a subtype of current class");
  }

  /// All good, update symbol table and return
  symbolTable->addElement(node->id(), idType);
  return Status::Ok();
}

Status ClassesDefinitionPass::visit(Context *context, ClassNode *node) {
  /*  context->setCurrentClassName(node->className());
    auto *symbolTable = context->symbolTable();
    auto *methodTable = context->methodTable();

    /// Set parent tables
    if (node->hasParentClass()) {
      auto *parentSymbolTable = context->symbolTable(node->parentClassName());
      auto *parentMethodTable = context->methodTable(node->parentClassName());
      symbolTable->setParentTable(parentSymbolTable);
      methodTable->setParentTable(parentMethodTable);
    }

    /// Add self to symbol table
    const auto classID = context->currentClassID();
    const ExprType classType{.typeID = classID, .isSelf = true};
    symbolTable->addElement("self", classType);

    /// Add attributes to symbol table
    for (auto attribute : node->attributes()) {
      auto status = attribute->visitNode(context, this);
      if (!status.isOk()) {
        return status;
      }
    }

    /// Add methods to symbol table
    for (auto method : node->methods()) {
      auto status = method->visitNode(context, this);
      if (!status.isOk()) {
        return status;
      }
    }*/

  /// All good, return
  return Status::Ok();
}

Status ClassesDefinitionPass::visit(Context *context, ProgramNode *node) {
  auto *classRegistry = context->classRegistry();
  const auto &classTree = classRegistry->classTree();

  /// Add classes to class registry
  for (auto classNode : node->classes()) {
    auto status = classRegistry->addClass(classNode);
    if (!status.isOk()) {
      return status;
    }
  }

  /// Validate inheritance tree and sort classes
  std::vector<IdentifierType> sortedClasses;
  std::unordered_set<IdentifierType> visitedNow, visitedAll;
  for (auto it = classTree.begin(); it != classTree.end(); ++it) {
    /// Parent node must be defined
    const auto &parent = it->first;
    if (!classRegistry->hasClass(parent)) {
      return GenericError("Error: parent class not defined");
    }

    /// Traverse the inheritance tree starting from the parent node
    auto statusCycles = detectCyclesInClassTree(classTree, parent, &visitedNow,
                                                &visitedAll, &sortedClasses);

    /// Early return if an error was detected
    if (!statusCycles.isOk()) {
      return statusCycles;
    }
  }
  std::reverse(sortedClasses.begin(), sortedClasses.end());

  /// Initialize symbol tables and method tables
  for (auto &classID : sortedClasses) {
    auto status = classRegistry->classNode(classID)->visitNode(context, this);
    if (!status.isOk()) {
      return status;
    }
  }

  /// All good, return ok
  return Status::Ok();
}

} // namespace cool