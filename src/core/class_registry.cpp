#include <cool/core/class_registry.h>
#include <cool/ir/class.h>

#include <unordered_set>

namespace cool {

namespace {

Status detectCyclesInClassTree(
    const std::unordered_map<IdentifierType, std::vector<IdentifierType>> &tree,
    IdentifierType parent, std::unordered_set<IdentifierType> *visitedNow,
    std::unordered_set<IdentifierType> *visitedAll) {
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
      auto status =
          detectCyclesInClassTree(tree, child, visitedNow, visitedAll);
      if (!status.isOk()) {
        return status;
      }
    }
  }

  /// Remove node from visited list and return
  visitedNow->erase(visitedNow->find(parent));
  return Status::Ok();
}

int32_t leastCommonAncestorImpl(
    IdentifierType root, IdentifierType descendantA, IdentifierType descendantB,
    const std::unordered_map<IdentifierType, std::vector<IdentifierType>>
        &classTree_,
    ExprType *result) {
  /// Handle leaf nodes
  if (classTree_.count(root) == 0) {
    return root == descendantA || root == descendantB ? 1 : 0;
  }

  /// Post-order traversal of the class tree
  int32_t count = root == descendantA || root == descendantB ? 1 : 0;
  for (auto child : classTree_.find(root)->second) {
    const int32_t subtreeCount = leastCommonAncestorImpl(
        child, descendantA, descendantB, classTree_, result);

    /// Solution found already, exit
    if (subtreeCount == 2) {
      return subtreeCount;
    }
    count += subtreeCount;
  }

  /// Least common ancestor found
  if (count == 2) {
    result->typeID = root;
  }
  return count;
}

} // namespace

ClassRegistry::ClassRegistry() {
  /// Register built-in classes
  std::vector<std::string> names = {"Object", "Int", "Bool"};
  for (auto &name : names) {
    classNamesToClassIDs_[name] = classNamesToClassIDs_.size();
    classRegistry_[classNamesToClassIDs_[name]] = nullptr;
  }
}

Status ClassRegistry::addClass(std::shared_ptr<ClassNode> node) {
  /// Class ID must have not been added to registry before
  IdentifierType classID = findOrCreateClassID(node->className());
  if (classRegistry_.count(classID) > 0) {
    return cool::GenericError("Error: class is already defined");
  }

  /// Get the parent class ID and add inheritance relationship
  const std::string parentClassName =
      node->hasParentClass() ? node->parentClassName() : "Object";
  IdentifierType parentClassID = findOrCreateClassID(parentClassName);
  classTree_[parentClassID].push_back(classID);

  /// Add class to registry and return
  classRegistry_[classID] = std::move(node);
  return Status::Ok();
}

Status ClassRegistry::checkInheritanceTree() const {
  /// All parent classes in the inheritance tree must have a definition
  for (auto &[parent, children] : classTree_) {
    if (classRegistry_.count(parent) == 0) {
      return cool::GenericError("Error: parent class is not defined");
    }
  }

  /// The inheritance tree must not contain cycles
  std::unordered_set<IdentifierType> visitedNow, visitedAll;
  for (auto it = classTree_.begin(); it != classTree_.end(); ++it) {
    /// Node already visited, skip it
    if (visitedAll.count(it->first)) {
      continue;
    }

    /// Traverse the inheritance tree from the current parent node
    auto status = detectCyclesInClassTree(classTree_, it->first, &visitedNow,
                                          &visitedAll);

    /// Early return if an error was detected
    if (!status.isOk()) {
      return status;
    }
  }

  /// All good, return ok status
  return Status::Ok();
}

ExprType ClassRegistry::leastCommonAncestor(const ExprType &descendantA,
                                            const ExprType &descendantB) const {
  ExprType result;
  leastCommonAncestorImpl(classNamesToClassIDs_.find("Object")->second,
                          descendantA.typeID, descendantB.typeID, classTree_,
                          &result);
  return result;
}

IdentifierType
ClassRegistry::findOrCreateClassID(const std::string &className) {
  if (classNamesToClassIDs_.count(className) == 0) {
    classNamesToClassIDs_[className] = classNamesToClassIDs_.size();
  }
  return classNamesToClassIDs_[className];
}

bool ClassRegistry::isAncestorOf(const ExprType &descendantType,
                                 const ExprType &ancestorType) const {
  auto descendant = descendantType.typeID;
  auto ancestor = ancestorType.typeID;

  /// Nothing to do if ancestor class does not have descendants
  if (classTree_.count(ancestor) == 0) {
    return false;
  }

  /// Examine the subtree rooted at ancestor, searching for descendant
  auto frontier = classTree_.find(ancestor)->second;
  while (frontier.size()) {
    auto root = frontier.back();
    frontier.pop_back();

    if (root == descendant) {
      return true;
    }

    auto kvIterator = classTree_.find(root);
    if (kvIterator == classTree_.end()) {
      continue;
    }

    for (auto child : kvIterator->second) {
      frontier.push_back(child);
    }
  }

  /// Descendant not found in subtree rooted at ancestor, return false
  return false;
}

} // namespace cool
