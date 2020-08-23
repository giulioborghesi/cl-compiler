namespace cool {

template <typename SymbolTableT, typename MethodTableT>
Context<SymbolTableT, MethodTableT>::Context(ClassRegistry *classRegistry)
    : classRegistry_(classRegistry), logger_(nullptr) {}

template <typename SymbolTableT, typename MethodTableT>
Context<SymbolTableT, MethodTableT>::Context(
    ClassRegistry *classRegistry, std::shared_ptr<LoggerCollection> logger)
    : classRegistry_(classRegistry), logger_(logger) {}

template <typename SymbolTableT, typename MethodTableT>
template <typename T>
void Context<SymbolTableT, MethodTableT>::initializeGenericTable(
    TableCollectionT<std::unique_ptr<T>> &tables) {
  const auto classID = classRegistry_->typeID(currentClassName_);
  assert(tables.count(classID) == 0);

  tables.insert({classID, std::make_unique<T>()});
  auto table = tables.find(classID)->second.get();

  /// Set parent table
  auto classNode = classRegistry_->classNode(classID);
  if (classNode->hasParentClass()) {
    auto parentID = classRegistry_->typeID(classNode->parentClassName());
    auto parentTable = tables.find(parentID)->second.get();
    table->setParentTable(parentTable);
  }
}

} // namespace cool
