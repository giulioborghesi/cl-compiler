#ifndef COOL_CORE_CLASS_REGISTRY_H
#define COOL_CORE_CLASS_REGISTRY_H

#include <cool/core/status.h>
#include <cool/ir/common.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace cool {

/// Forward declaration
class ClassNode;

/// Class that implements a registry for class names
class ClassRegistry {

public:
  ClassRegistry();

  /// Add a class to the registry
  ///
  /// \param[in] node shared pointer to the class node to add
  /// \return cool::Status::Ok() if successfull, error message otherwise
  Status addClass(std::shared_ptr<ClassNode> node);

  /// Traverse the class inheritance tree and check for errors
  ///
  /// \return cool::Status::Ok() if the class inheritance tree is error free
  Status checkInheritanceTree() const;

  /// Return the identifier corresponding to a class name
  ///
  /// \param[in] className class name
  /// \return the identifier corresponding to the input class name
  IdentifierType typeID(const std::string &className) const {
    auto it = classNamesToClassIDs_.find(className);
    if (it == classNamesToClassIDs_.end()) {
      return -1;
    }
    return it->second;
  }

  /// Find the least common ancestors of two classes
  ///
  /// \note This method requires both descendants to be registered. It also
  /// assumes that the class inheritance tree is well formed. It is the client
  /// code responsibility to check that these preconditions are satisfied
  ///
  /// \param[in] firstDescendantType type of first descendant class
  /// \param[in] secondDescendantType type of second descendant class
  /// \return the name of the least common ancestor class
  ExprType leastCommonAncestor(const ExprType &firstDescendantType,
                               const ExprType &secondDescendantType) const;

  /// Check whether a class is a descendant of another class
  ///
  /// \param[in] descendantType descendant class type
  /// \param[in] ancestorType ancestor class type
  /// \return true if descendant is a descendant of ancestor, false otherwise
  bool isAncestorOf(const ExprType &descendantType,
                    const ExprType &ancestorType) const;

private:
  /// Find the ID of a class, if it exists, or create a new one
  ///
  /// This class will a new item to the class names / IDs dictionary if the
  /// class has not been assigned an ID yet
  ///
  /// \param[in] className class name
  /// \return the class ID
  IdentifierType findOrCreateClassID(const std::string &className);

  std::unordered_map<std::string, IdentifierType> classNamesToClassIDs_;
  std::unordered_map<IdentifierType, std::shared_ptr<ClassNode>> classRegistry_;
  std::unordered_map<IdentifierType, std::vector<IdentifierType>> classTree_;
};

} // namespace cool

#endif
