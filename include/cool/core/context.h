#ifndef COOL_CORE_CONTEXT_H
#define COOL_CORE_CONTEXT_H

#include <cool/core/class_registry.h>

#include <memory>

namespace cool {

/// Class that represents the context of a compiler pass / analysis
class Context {

public:
  Context() = delete;
  Context(ClassRegistry *classRegistry);

  /// Get the class registry
  ///
  /// \return a pointer to the class registry
  ClassRegistry *classRegistry() const { return classRegistry_.get(); }

  /// Get the name of the current class being processed
  ///
  /// \return the name of the current class being processed
  const std::string &currentClassName() const { return currentClassName_; }

  /// Set the name of the current class being processed
  ///
  /// \param[in] currentClassName name of the current class being processed
  void setCurrentClassName(const std::string &currentClassName) {
    currentClassName_ = currentClassName;
  }

private:
  std::string currentClassName_;
  std::unique_ptr<ClassRegistry> classRegistry_;
};

} // namespace cool

#endif
