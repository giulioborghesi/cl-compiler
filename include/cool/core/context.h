#ifndef COOL_CORE_CONTEXT_H
#define COOL_CORE_CONTEXT_H

#include <cool/core/class_registry.h>
#include <cool/core/symbol_table.h>
#include <cool/ir/common.h>

#include <memory>

namespace cool {

/// Class that represents the context of a compiler pass / analysis
class Context {

  using KeyT = std::string;
  using ValueT = ExprType;
  using SymbolTableT = SymbolTable<KeyT, ValueT>;

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

  /// Get the symbol table for the class being processed
  ///
  /// \return the symbol table for the class being processed
  SymbolTableT &symbolTable() { return symbolTables_[currentClassName_]; }

private:
  std::string currentClassName_;
  std::unique_ptr<ClassRegistry> classRegistry_;
  std::unordered_map<std::string, SymbolTableT> symbolTables_;
};

} // namespace cool

#endif
