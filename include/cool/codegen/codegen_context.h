#ifndef COOL_CODEGEN_CODEGEN_CONTEXT_H
#define COOL_CODEGEN_CODEGEN_CONTEXT_H

#include <cool/core/context.h>
#include <cool/core/symbol_table.h>

#include <sstream>

namespace cool {

/// \brief Helper struct to store the information needed about an identifier in
/// a codegen context
struct IdentifierCodegenInfo {
  const bool isAttribute = false;
  const int32_t position = -1;
  IdentifierCodegenInfo(bool inpIsAttribute, const int32_t inpPosition)
      : isAttribute(inpIsAttribute), position(inpPosition) {}
};

/// \brief Helper struct to store the information needed about a method in a
/// codegen context
struct MethodCodegenInfo {
  const int32_t position = -1;
  MethodCodegenInfo(const int32_t inpPosition) : position(inpPosition) {}
};

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

  /// \brief Generate a label
  ///
  /// \note The generated label will be appended with an integer representing
  /// the number of same-prefix labels generated so far
  ///
  /// \param[in] prefix label prefix
  /// \return a new label with the given prefix
  std::string generateLabel(const std::string &prefix) {
    std::stringstream label;
    label << prefix << "_" << labels_.count(prefix);
    labels_[prefix] += 1;
    return label.str();
  }

  /// \brief Increment stack size by count elements
  ///
  /// \param[in] count size to add to stack size
  void incrementStackSize(const int32_t count) { stackSize_ += count; }

  /// \brief Decrement stack size by count elements
  ///
  /// \param[in] count size to subtract from stack size
  void decrementStackSize(const int32_t count) {
    assert(count >= stackSize_);
    stackSize_ -= count;
  }

  /// \brief Reset the stack size to zero
  void resetStackSize() { stackSize_ = 0; }

  /// \brief Get the stack size
  ///
  /// \return the stack size
  int32_t stackSize() const { return stackSize_; }

private:
  int32_t stackSize_;
  std::unordered_map<std::string, size_t> labels_;
};

} // namespace cool

#endif
