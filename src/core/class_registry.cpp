#include <cool/core/class_registry.h>
#include <cool/ir/class.h>

#include <unordered_set>

namespace cool {

Status ClassRegistry::addClass(std::shared_ptr<ClassNode> node) {
  /// Class ID must have not been added to registry before
  IdentifierType classID = findOrCreateClassID(node->className());
  if (classRegistry_.count(classID) > 0) {
    return cool::GenericError("Error: class is already defined");
  }

  /// Add class to registry and return
  classRegistry_[classID] = std::move(node);
  return Status::Ok();
}

const std::string &
ClassRegistry::className(const IdentifierType classID) const {
  auto it = classRegistry_.find(classID);
  assert(it != classRegistry_.end());
  return it->second->className();
}

uint32_t ClassRegistry::distanceToRoot(const IdentifierType &classID) const {
  auto tailClassNode = classRegistry_.find(classID)->second;

  uint32_t distance = 0;
  while (tailClassNode->hasParentClass()) {
    ++distance;
    auto className = tailClassNode->parentClassName();
    auto classID = namesToIDs_.find(className)->second;
    tailClassNode = classRegistry_.find(classID)->second;
  }

  return distance;
}

ExprType ClassRegistry::leastCommonAncestor(const ExprType &descendantA,
                                            const ExprType &descendantB) const {
  /// Least common ancestor of identical types is the type itself
  if (descendantA == descendantB) {
    return descendantA;
  }

  auto aDistance = distanceToRoot(descendantA.typeID);
  auto bDistance = distanceToRoot(descendantB.typeID);

  if (aDistance < bDistance) {
    return leastCommonAncestor(descendantB, descendantA);
  }

  auto tailClassNodeA = classRegistry_.find(descendantA.typeID)->second;
  auto tailClassNodeB = classRegistry_.find(descendantB.typeID)->second;

  while (aDistance > bDistance) {
    --aDistance;
    auto className = tailClassNodeA->parentClassName();
    auto classID = namesToIDs_.find(className)->second;
    tailClassNodeA = classRegistry_.find(classID)->second;
  }

  while (tailClassNodeA != tailClassNodeB) {
    {
      auto className = tailClassNodeB->parentClassName();
      auto classID = namesToIDs_.find(className)->second;
      tailClassNodeA = classRegistry_.find(classID)->second;
    }

    {
      auto className = tailClassNodeB->parentClassName();
      auto classID = namesToIDs_.find(className)->second;
      tailClassNodeB = classRegistry_.find(classID)->second;
    }
  }

  const auto typeID = namesToIDs_.find(tailClassNodeA->className())->second;
  return ExprType{.typeID = typeID, .isSelf = false};
}

IdentifierType
ClassRegistry::findOrCreateClassID(const std::string &className) {
  if (namesToIDs_.count(className) == 0) {
    namesToIDs_[className] = namesToIDs_.size();
  }
  return namesToIDs_[className];
}

bool ClassRegistry::conformTo(const ExprType &childType,
                              const ExprType &parentType) const {
  // Special treatment needed for SELF_TYPE
  if (parentType.isSelf) {
    return childType.isSelf && (childType.typeID == parentType.typeID);
  }

  auto childDistance = distanceToRoot(childType.typeID);
  const auto parentDistance = distanceToRoot(parentType.typeID);

  if (childDistance < parentDistance) {
    return false;
  }

  auto tailClassNode = classRegistry_.find(childType.typeID)->second;
  while (childDistance > parentDistance) {
    --childDistance;
    auto className = tailClassNode->parentClassName();
    auto classID = namesToIDs_.find(className)->second;
    tailClassNode = classRegistry_.find(classID)->second;
  }

  auto targetID = namesToIDs_.find(tailClassNode->className())->second;
  if (parentType.typeID == targetID) {
    return true;
  }
  return false;
}

} // namespace cool
