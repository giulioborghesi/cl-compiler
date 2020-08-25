#ifndef COOL_CORE_CONTEXT_H
#define COOL_CORE_CONTEXT_H

#include <cool/core/class_registry.h>
#include <cool/ir/common.h>

#include <memory>

namespace cool {

/// Forward declaration
class LoggerCollection;

/// Class that represents the context of a compiler pass / analysis
template <typename SymbolTableT, typename MethodTableT> class Context {

  template <typename T>
  using TableCollectionT = std::unordered_map<IdentifierType, T>;

public:
  Context() = delete;
  explicit Context(std::shared_ptr<ClassRegistry> classRegistry);

  Context(std::shared_ptr<ClassRegistry> classRegistry,
          std::shared_ptr<LoggerCollection> logger);

  ~Context() = default;

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

  /// Initialize symbol and method tables
  void initializeTables() {
    initializeGenericTable(symbolTables_);
    initializeGenericTable(methodTables_);
  }

  /// Get the logger
  ///
  /// \return a pointer to the logger
  LoggerCollection *logger() const { return logger_.get(); }

  /// Get or create the method table for the currently active class
  ///
  /// \return the method table for the currently active class
  MethodTableT *methodTable() { return methodTable(currentClassName_); }

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

  /// Get or create the symbol table for the currently active class
  ///
  /// \return the symbol table for the currently active class
  SymbolTableT *symbolTable() { return symbolTable(currentClassName_); }

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
  /// Initialize a table for the currently active class
  ///
  /// \param[in] tables tables collection
  template <typename T>
  void initializeGenericTable(TableCollectionT<std::unique_ptr<T>> &tables);

  std::string currentClassName_;
  std::shared_ptr<ClassRegistry> classRegistry_;
  std::shared_ptr<LoggerCollection> logger_;

  TableCollectionT<std::unique_ptr<SymbolTableT>> symbolTables_;
  TableCollectionT<std::unique_ptr<MethodTableT>> methodTables_;
};

} // namespace cool

#include <cool/core/context.inl>

#endif
