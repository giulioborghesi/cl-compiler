#ifndef COOL_CODEGEN_CODEGEN_CONTEXT_H
#define COOL_CODEGEN_CODEGEN_CONTEXT_H

#include <cool/core/context.h>
#include <cool/core/symbol_table.h>

namespace cool {

/// \brief Helper struct to store the information needed about an identifier in
/// a codegen context
struct IdentifierCodegenInfo {
  bool isAttribute = false;
  int32_t position = 0;
  IdentifierCodegenInfo(bool inpIsAttribute, const int32_t inpPosition)
      : isAttribute(inpIsAttribute), position(inpPosition) {}
};

struct MethodCodegenInfo {};

class CodegenContext
    : public Context<SymbolTable<std::string, IdentifierCodegenInfo>,
                     SymbolTable<std::string, MethodCodegenInfo>> {

public:
  CodegenContext() = delete;
  explicit CodegenContext(ClassRegistry *classRegistry)
      : Context(classRegistry) {}

  CodegenContext(ClassRegistry *classRegistry,
                 std::shared_ptr<LoggerCollection> logger)
      : Context(classRegistry, logger) {}

  /// Increment stack size by count elements
  ///
  /// \param[in] count size to add to stack size
  void incrementStackSize(const size_t count) { stackSize_ += count; }

  /// Decrement stack size by count elements
  ///
  /// \param[in] count size to subtract from stack size
  void decrementStackSize(const size_t count) {
    assert(count >= stackSize_);
    stackSize_ -= count;
  }

  /// Reset the stack size to zero
  void resetStackSize() { stackSize_ = 0; }

  /// Get the stack size
  ///
  /// \return the stack size
  size_t stackSize() const { return stackSize_; }

private:
  size_t stackSize_;
};

} // namespace cool

#endif
