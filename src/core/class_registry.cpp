#include <cool/core/class_registry.h>
#include <cool/ir/class.h>

namespace cool {

Status ClassRegistry::addClass(std::shared_ptr<ClassNode> node) {
  /// Classes cannot be redefined
  if (classRegistry_.count(node->className()) > 0) {
    return cool::GenericError("Error: class is already defined");
  }

  /// Add class inheritance relationship to class inheritance tree
  const std::string parentClassName =
      node->hasParentClass() ? node->parentClassName() : "Object";
  classInheritanceTree_[parentClassName].push_back(node->className());

  /// Add class to registry and return
  classRegistry_[node->className()] = std::move(node);
  return Status::Ok();
}

Status ClassRegistry::checkInheritanceTree() const {
  /// All parent classes in the inheritance tree must have a definition
  for (auto &[parent, children] : classInheritanceTree_) {
    if (classRegistry_.count(parent) == 0) {
      return cool::GenericError("Error: parent class is not defined");
    }
  }

  /// The inheritance tree must not contain cycles
  std::unordered_set<std::string> visited;
  return detectInheritanceTreeCycles("Object", visited);
}

Status ClassRegistry::detectInheritanceTreeCycles(
    const std::string &parent, std::unordered_set<std::string> &visited) const {
  /// Check for cyclic dependency in the inheritance tree
  if (visited.count(parent)) {
    return cool::GenericError("Error: cyclic class dependency detected");
  }
  visited.insert(parent);

  /// Recurse on children nodes
  auto kvIterator = classInheritanceTree_.find(parent);
  if (kvIterator != classInheritanceTree_.end()) {
    for (auto child : kvIterator->second) {
      auto status = detectInheritanceTreeCycles(child, visited);
      if (!status.isOk()) {
        return status;
      }
    }
  }

  /// Remove node from visited list and return
  visited.erase(visited.find(parent));
  return cool::Status::Ok();
}

} // namespace cool
