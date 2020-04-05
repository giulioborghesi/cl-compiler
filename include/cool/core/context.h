#ifndef COOL_CORE_CONTEXT_H
#define COOL_CORE_CONTEXT_H

#include <cool/analysis/method_record.h>
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

  using MethodValueT = MethodRecord;
  using MethodTableT = SymbolTable<KeyT, MethodValueT>;

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

  /// Get the id of the current class being processed
  ///
  /// \return the ID of the current class being processed
  const IdentifierType currentClassID() const {
    return classRegistry_->typeID(currentClassName_);
  }

  /// Get the method table for the specified type
  ///
  /// \param[in] type class type
  /// \return the method table for the specified type
  MethodTableT *methodTable(const ExprType type) const {
    const auto typeID = type.typeID;
    if (methodTables_.find(typeID) == methodTables_.end()) {
      return nullptr;
    }
    return methodTables_.find(typeID)->second.get();
  }

  /// Set the name of the current class being processed
  ///
  /// \param[in] currentClassName name of the current class being processed
  void setCurrentClassName(const std::string &currentClassName) {
    currentClassName_ = currentClassName;
  }

  /// Get the symbol table for the class being processed
  ///
  /// \return the symbol table for the class being processed
  SymbolTableT *symbolTable() {
    const auto currentClassID = classRegistry_->typeID(currentClassName_);
    if (!symbolTables_.count(currentClassID)) {
      symbolTables_.insert({currentClassID, std::make_unique<SymbolTableT>()});
    }
    return symbolTables_.find(currentClassID)->second.get();
  }

private:
  std::string currentClassName_;
  std::unique_ptr<ClassRegistry> classRegistry_;

  template <typename T>
  using TableCollectionT = std::unordered_map<IdentifierType, T>;

  TableCollectionT<std::unique_ptr<SymbolTableT>> symbolTables_;
  TableCollectionT<std::unique_ptr<MethodTableT>> methodTables_;
};

} // namespace cool

#endif
