
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
void Context::initializeGenericTable(
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

template void Context::initializeGenericTable<Context::SymbolTableT>(
    TableCollectionT<std::unique_ptr<Context::SymbolTableT>> &tables);

template void Context::initializeGenericTable<Context::MethodTableT>(
    TableCollectionT<std::unique_ptr<Context::MethodTableT>> &tables);

} // namespace cool
