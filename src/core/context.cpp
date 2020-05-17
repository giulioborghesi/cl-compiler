
#include <cool/core/context.h>
#include <cool/core/logger_collection.h>
#include <cool/ir/class.h>

namespace cool {

Context::Context(ClassRegistry *classRegistry)
    : classRegistry_(classRegistry), logger_(nullptr) {}

Context::Context(ClassRegistry *classRegistry,
                 std::shared_ptr<LoggerCollection> logger)
    : classRegistry_(classRegistry), logger_(logger) {}

template <typename T>
T *Context::genericTable(TableCollectionT<std::unique_ptr<T>> &tables) {
  const auto classID = classRegistry_->typeID(currentClassName_);
  if (tables.count(classID)) {
    return tables.find(classID)->second.get();
  }

  /// Table does not exist, create it
  tables.insert({classID, std::make_unique<T>()});
  auto table = tables.find(classID)->second.get();

  /// Set parent table
  auto classNode = classRegistry_->classNode(classID);
  if (classNode->hasParentClass()) {
    auto parentID = classRegistry_->typeID(classNode->parentClassName());
    auto parentTable = tables.find(parentID)->second.get();
    table->setParentTable(parentTable);
  }
  return table;
}

template Context::SymbolTableT *Context::genericTable<Context::SymbolTableT>(
    TableCollectionT<std::unique_ptr<Context::SymbolTableT>> &tables);

template Context::MethodTableT *Context::genericTable<Context::MethodTableT>(
    TableCollectionT<std::unique_ptr<Context::MethodTableT>> &tables);

} // namespace cool
