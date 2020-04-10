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

  /// Get the method table for the currently active class
  ///
  /// \note This method will create a new table for the currently active class
  /// if one does not exist yet
  ///
  /// \return the method table for the currently active class
  MethodTableT *methodTable() {
    const auto currentClassID = classRegistry_->typeID(currentClassName_);
    if (!methodTables_.count(currentClassID)) {
      methodTables_.insert({currentClassID, std::make_unique<MethodTableT>()});
    }
    return methodTables_.find(currentClassID)->second.get();
  }

  /// Get the method table for the specified class given its name
  ///
  /// \warning This method assumes that the specified class exists
  ///
  /// \param[in] className class name
  /// \return the method table for the specified class
  MethodTableT *methodTable(const std::string &className) const {
    const auto classID = classRegistry_->typeID(className);
    auto it = methodTables_.find(classID);
    assert(it != methodTables_.end());
    return it->second.get();
  }

  /// Get the method table for the specified class given its type ID
  ///
  /// \warning This method assumes that the specified class exists
  ///
  /// \param[in] typeID type ID
  /// \return the method table for the specified class
  MethodTableT *methodTable(const IdentifierType &typeID) const {
    auto it = methodTables_.find(typeID);
    assert(it != methodTables_.end());
    return it->second.get();
  }

  /// Set the name of the current class being processed
  ///
  /// \param[in] currentClassName name of the current class being processed
  void setCurrentClassName(const std::string &currentClassName) {
    currentClassName_ = currentClassName;
  }

  /// Get the symbol table for the currently active class
  ///
  /// \return the symbol table for the currently active class
  SymbolTableT *symbolTable() {
    const auto currentClassID = classRegistry_->typeID(currentClassName_);
    if (!symbolTables_.count(currentClassID)) {
      symbolTables_.insert({currentClassID, std::make_unique<SymbolTableT>()});
    }
    return symbolTables_.find(currentClassID)->second.get();
  }

  /// Get the symbol table for the specified class
  ///
  /// \warning This method assumes that the specified class exists
  ///
  /// \param[in] className class name
  /// \return the symbol table for the specified class
  SymbolTableT *symbolTable(const std::string &className) const {
    const auto classID = classRegistry_->typeID(className);
    auto it = symbolTables_.find(classID);
    assert(it != symbolTables_.end());
    return it->second.get();
  }

  /// Get the symbol table for the specified class given its ID
  ///
  /// \warning This method assumes that the specified class exists
  ///
  /// \param[in] typeID type ID
  /// \return the symbol table for the specified class
  SymbolTableT *symbolTable(const IdentifierType &typeID) const {
    auto it = symbolTables_.find(typeID);
    assert(it != symbolTables_.end());
    return it->second.get();
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
