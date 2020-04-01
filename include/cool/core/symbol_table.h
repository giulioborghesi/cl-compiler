#ifndef COOL_CORE_SYMBOL_TABLE_H
#define COOL_CORE_SYMBOL_TABLE_H

#include <cool/ir/common.h>

#include <string>
#include <unordered_map>
#include <vector>

namespace cool {

/// Class that implements a nested symbol table. The symbol table will always
/// have a class scope to store the class attributes. The symbol table also
/// holds a pointer to a parent table to handle the case in which a class
/// inherits from a parent class. The nested scopes are used only while
/// processing a class: once a class is processed, only the class scope
/// will remain active.
template <typename KeyT, typename ValueT> class SymbolTable {

public:
  SymbolTable();

  /// Add symbol to current scope
  ///
  /// \param[in] key: key of the element to add
  /// \param[in] value: value of the element to add
  void addElement(const KeyT &key, const ValueT &value);

  /// Enter a new nested scope
  void enterScope();

  /// Exit the current nested scope
  ///
  /// This function call will throw if an attempt of exiting the class scope is
  /// detected
  void exitScope();

  /// Check whether the input key is defined in the current scope
  ///
  /// \param[in] key: key of the element to check
  /// \return true if the element is in the current scope, false therwise
  bool findKeyInScope(const KeyT &key) const;

  /// Check whether the input key is defined in the nested table
  ///
  /// \param[in] key: key of the element to check
  /// \return true if the element is in the table, false otherwise
  bool findKeyInTable(const KeyT &key) const;

  /// Set the parent symbol table. Used for handling inheritance
  ///
  /// \param[in] parent pointer to parent table
  void setParent(SymbolTable *parent);

  /// Return the value associated with the input key. The key must be present in
  /// the table, otherwise a runtime assertion will be triggered
  ///
  /// \param[in] key: key of the element to return
  /// \return the value associated with the given key.
  const ValueT &get(const KeyT &key) const;

private:
  std::vector<std::unordered_map<KeyT, ValueT>> nestedTables_;
  SymbolTable *parentTable_;
};

} // namespace cool

#endif

#include <cool/core/symbol_table.inl>
