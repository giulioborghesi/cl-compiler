#ifndef COOL_UTILS_TABLES_H
#define COOL_UTILS_TABLES_H

#include <cool/ir/common.h>

#include <string>
#include <unordered_map>
#include <vector>

namespace cool {

/// Class that implements a symbol table
template <typename KeyT, typename ValueT,
          typename TableT = std::unordered_map<KeyT, ValueT>>
class NestedTable {

public:
  NestedTable() = default;

  /// Enter a new nested scope
  void enterScope();

  /// Add symbol to current scope
  ///
  /// \param[in] key: key of the element to add
  /// \param[in] value: value of the element to add
  void addElement(const KeyT &key, const ValueT &value);

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

  /// Return the value associated with the input key. The key must be present in
  /// the table
  ///
  /// \param[in] key: key of the element to return
  /// \return the value associated with the given key.
  const ValueT &returnElement(const KeyT &key) const;

  /// Exit the current nested scope
  /// \return true if the operation is succesfull, false otherwise
  bool exitScope();

private:
  std::vector<TableT> nestedTables_;
};

/// Alias type for a symbol table
using SymbolTable = NestedTable<std::string, ExprT>;

} // namespace cool

#endif
