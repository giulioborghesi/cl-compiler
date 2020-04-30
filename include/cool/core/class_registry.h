#ifndef COOL_CORE_CLASS_REGISTRY_H
#define COOL_CORE_CLASS_REGISTRY_H

#include <cool/core/status.h>
#include <cool/ir/common.h>
#include <cool/ir/fwd.h>

#include <cassert>
#include <string>
#include <unordered_map>
#include <vector>

namespace cool {

/// Class that implements a registry for class names
class ClassRegistry {

public:
  ClassRegistry() = default;

  /// \brief Add a class to the registry
  ///
  /// \param[in] node shared pointer to the class node to add
  /// \return cool::Status::Ok() if successfull, error message otherwise
  Status addClass(ClassNodePtr node);

  /// \brief Get a class node from the registry given its ID.
  ///
  /// \note This method will trigger an assertion if the class ID is invalid
  ///
  /// \param[in] classID class ID
  /// \return a shared pointer to the class node
  ClassNodePtr classNode(const IdentifierType &classID) const {
    assert(classRegistry_.count(classID) > 0);
    return classRegistry_.find(classID)->second;
  }

  /// \brief Get a class node from the registry given its name.
  ///
  /// \note This method will trigger an assertion if the class name is invalid
  ///
  /// \param[in] className class name
  /// \return a shared pointer to the class node
  ClassNodePtr classNode(const std::string &className) const {
    return classRegistry_.find(this->typeID(className))->second;
  }

  /// \brief Return the identifier corresponding to a class name
  ///
  /// \param[in] className class name
  /// \return the identifier corresponding to the input class name
  IdentifierType typeID(const std::string &className) const {
    auto it = namesToIDs_.find(className);
    assert(it != namesToIDs_.end());
    return it->second;
  }

  /// \brief Return the class name corresponding to a type identifier
  ///
  /// \param[in] classID class ID
  /// \return the class name corresponding to the input class ID
  const std::string &className(const IdentifierType classID) const;

  /// \brief Determine whether a class is in the registry
  ///
  /// \param[in] className class name
  /// \return true if the class is in the registry, false otherwise
  bool hasClass(const std::string &className) const {
    return namesToIDs_.count(className) > 0;
  }

  /// \brief Overload that uses the class identifier type
  ///
  /// \param[in] classID class ID
  /// \return true if the class is in the registry, false otherwise
  bool hasClass(const IdentifierType &classID) const {
    return classRegistry_.count(classID) > 0;
  }

  /// \brief Find the least common ancestors of two types
  ///
  /// \note This method requires both descendants to be registered. It also
  /// assumes that the class inheritance tree is well formed. It is the
  /// client code responsibility to check that these preconditions are
  /// satisfied
  ///
  /// \param[in] firstDescendantType type of first descendant class
  /// \param[in] secondDescendantType type of second descendant class
  /// \return the name of the least common ancestor class
  ExprType leastCommonAncestor(const ExprType &firstDescendantType,
                               const ExprType &secondDescendantType) const;

  /// \brief Check whether a type conforms to another type
  ///
  /// \note This algorithm could be optimize to run in constant time. The
  /// current implementation runs in linear time.
  ///
  /// \param[in] childType child  type
  /// \param[in] parentType parent type
  /// \return true if child type conforms to parent type
  bool conformTo(const ExprType &childType, const ExprType &parentType) const;

private:
  /// \brief Find the ID of a class, if it exists, or create a new one
  ///
  /// This class will a new item to the class names / IDs dictionary if the
  /// class has not been assigned an ID yet
  ///
  /// \param[in] className class name
  /// \return the class ID
  IdentifierType findOrCreateClassID(const std::string &className);

  /// \brief Find the distance to the base superclass
  ///
  /// \param[in] classID class ID
  /// \return the distance to the base class
  uint32_t distanceToRoot(const IdentifierType &classID) const;

  /// Dictionaries
  std::unordered_map<std::string, IdentifierType> namesToIDs_;
  std::unordered_map<IdentifierType, ClassNodePtr> classRegistry_;
};

} // namespace cool

#endif
