#ifndef COOL_CORE_CLASS_REGISTRY_H
#define COOL_CORE_CLASS_REGISTRY_H

#include <cool/core/status.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace cool {

/// Forward declaration
class ClassNode;

/// Class that implements a registry for class names
class ClassRegistry {

public:
  ClassRegistry() = default;

  /// Add a class to the registry
  ///
  /// \param[in] node shared pointer to the class node to add
  /// \return cool::Status::Ok() if successfull, error message otherwise
  Status addClass(std::shared_ptr<ClassNode> node);

  /// Traverse the class inheritance tree and check for errors
  ///
  /// \return cool::Status::Ok() if the class inheritance tree does not contain
  /// errors
  Status checkInheritanceTree() const;

private:
  Status
  detectInheritanceTreeCycles(const std::string &parent,
                              std::unordered_set<std::string> &visited) const;

  template <typename T, typename U> using MapT = std::unordered_map<T, U>;

  MapT<std::string, std::shared_ptr<ClassNode>> classRegistry_;
  MapT<std::string, std::vector<std::string>> classInheritanceTree_;
};

} // namespace cool

#endif
