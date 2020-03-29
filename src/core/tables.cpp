#include <cool/utils/tables.h>

namespace cool {

template <typename KeyT, typename ValueT, typename TableT>
void NestedTable<KeyT, ValueT, TableT>::enterScope() {
  nestedTables_.emplace_back();
}

template <typename KeyT, typename ValueT, typename TableT>
bool NestedTable<KeyT, ValueT, TableT>::findKeyInScope(const KeyT &key) const {
  if (nestedTables_.size() == 0) {
    return false;
  }
  return nestedTables_.back().count(key) > 0;
}

template <typename KeyT, typename ValueT, typename TableT>
bool NestedTable<KeyT, ValueT, TableT>::findKeyInTable(const KeyT &key) const {
  for (auto it = nestedTables_.rbegin(); it != nestedTables_.rend(); ++it) {
    if (it->count(key) > 0) {
      return true;
    }
  }
  return false;
}

template <typename KeyT, typename ValueT, typename TableT>
const ValueT &
NestedTable<KeyT, ValueT, TableT>::returnElement(const KeyT &key) const {
  assert(nestedTables_.size() > 0);
  for (auto it = nestedTables_.rbegin(); it != nestedTables_.rend(); ++it) {
    if (it->count(key) > 0) {
      return (*it)[key];
    }
  }
  assert(0);
  return ValueT{};
}

template <typename KeyT, typename ValueT, typename TableT>
bool NestedTable<KeyT, ValueT, TableT>::exitScope() {
  if (nestedTables_.size() == 0) {
    return false;
  }
  nestedTables_.pop_back();
  return true;
}

} // namespace cool
