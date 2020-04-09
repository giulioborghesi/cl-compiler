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
  std::vector<std::string> names = {"Object", "Int", "Bool", "String", "IO"};
  for (auto &name : names) {
    auto classID = findOrCreateClassID(name);
    classRegistry_[classID] = nullptr;
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
  leastCommonAncestorImpl(namesToIDs_.find("Object")->second,
                          descendantA.typeID, descendantB.typeID, classTree_,
                          &result);
  return result;
}

IdentifierType
ClassRegistry::findOrCreateClassID(const std::string &className) {
  if (namesToIDs_.count(className) == 0) {
    namesToIDs_[className] = namesToIDs_.size();
    IDsToNames_[namesToIDs_[className]] = className;
  }
  return namesToIDs_[className];
}

bool ClassRegistry::conformTo(const ExprType &childType,
                              const ExprType &parentType) const {
  auto child = childType.typeID;
  auto parent = parentType.typeID;

  /// Nothing to do if parent type does not have descendants
  if (classTree_.count(parent) == 0) {
    return false;
  }

  /// Examine the subtree rooted at ancestor, searching for descendant
  auto frontier = classTree_.find(parent)->second;
  while (frontier.size()) {
    auto root = frontier.back();
    frontier.pop_back();

    if (root == child) {
      return true;
    }

    auto kvIterator = classTree_.find(root);
    if (kvIterator == classTree_.end()) {
      continue;
    }

    for (auto newChild : kvIterator->second) {
      frontier.push_back(newChild);
    }
  }

  /// Descendant not found in subtree rooted at ancestor, return false
  return false;
}

} // namespace cool
