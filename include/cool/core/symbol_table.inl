namespace cool {

template <typename KeyT, typename ValueT>
SymbolTable<KeyT, ValueT>::SymbolTable() : parentTable_(nullptr) {
  this->enterScope();
}

template <typename KeyT, typename ValueT>
Status SymbolTable<KeyT, ValueT>::addElement(const KeyT &key,
                                             const ValueT &value) {
  if (this->findKeyInScope(key)) {
    return GenericError("Error: identifier already defined in current scope");
  }
  nestedTables_.back()[key] = value;
  return Status::Ok();
}

template <typename KeyT, typename ValueT>
void SymbolTable<KeyT, ValueT>::enterScope() {
  nestedTables_.emplace_back();
}

template <typename KeyT, typename ValueT>
bool SymbolTable<KeyT, ValueT>::findKeyInScope(const KeyT &key) const {
  return nestedTables_.back().count(key) > 0;
}

template <typename KeyT, typename ValueT>
bool SymbolTable<KeyT, ValueT>::findKeyInTable(const KeyT &key) const {
  /// Search in current symbol table
  for (auto it = nestedTables_.rbegin(); it != nestedTables_.rend(); ++it) {
    if (it->count(key) > 0) {
      return true;
    }
  }

  /// Search in parent symbol table if defined
  if (parentTable_) {
    return parentTable_->findKeyInTable(key);
  }

  /// Key not found, return false
  return false;
}

template <typename KeyT, typename ValueT>
const ValueT &SymbolTable<KeyT, ValueT>::get(const KeyT &key) const {
  /// Search in current symbol table
  for (auto it = nestedTables_.rbegin(); it != nestedTables_.rend(); ++it) {
    if (it->count(key) > 0) {
      return it->find(key)->second;
    }
  }

  /// Search in parent symbol table if defined
  if (parentTable_) {
    return parentTable_->get(key);
  }

  /// Key not found, trigger runtime assertion
  assert(0);
}

template <typename KeyT, typename ValueT>
void SymbolTable<KeyT, ValueT>::exitScope() {
  assert(nestedTables_.size() > 1);
  nestedTables_.pop_back();
}

} // namespace cool
