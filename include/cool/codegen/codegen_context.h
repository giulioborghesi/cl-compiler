#ifndef COOL_CODEGEN_CODEGEN_CONTEXT_H
#define COOL_CODEGEN_CODEGEN_CONTEXT_H

#include <cool/core/context.h>
#include <cool/core/symbol_table.h>

#include <sstream>
#include <unordered_set>

namespace cool {

/// \brief Helper struct to store the information needed about an identifier in
/// a codegen context
struct IdentifierCodegenInfo {
  const bool isAttribute = false;
  const int32_t position = -1;
  IdentifierCodegenInfo(bool inpIsAttribute, const int32_t inpPosition)
      : isAttribute(inpIsAttribute), position(inpPosition) {}
};

struct MethodCodegenInfo {
  std::string className;
  size_t position = 0;
  MethodCodegenInfo(const std::string &className, const int32_t position)
      : className(className), position(position) {}
};

class MethodTable {

  using KeyT = std::string;
  using ValueT = MethodCodegenInfo;
  using StorageT = std::unordered_map<KeyT, ValueT>;

public:
  bool findKey(const KeyT &key) const { return storage_.count(key); }

  const ValueT &get(const KeyT &key) const {
    assert(storage_.count(key));
    return storage_.find(key)->second;
  }

  void addElement(const KeyT &key, const ValueT &value) {
    if (storage_.find(key) != storage_.end()) {
      storage_.erase(key);
    }
    storage_.insert({key, value});
  }

  typename StorageT::iterator begin() { return storage_.begin(); }

  typename StorageT::iterator end() { return storage_.end(); }

  /// Set the parent symbol table. Used for handling inheritance
  ///
  /// \param[in] parentTable pointer to parent table
  void setParentTable(MethodTable *parentTable) {
    storage_ = parentTable->storage_;
  }

  size_t count() const { return storage_.size(); }

private:
  std::unordered_map<KeyT, ValueT> storage_;
};

class CodegenContext
    : public Context<SymbolTable<std::string, IdentifierCodegenInfo>,
                     MethodTable> {

public:
  CodegenContext() = delete;
  explicit CodegenContext(std::shared_ptr<ClassRegistry> classRegistry)
      : Context(classRegistry) {}

  CodegenContext(std::shared_ptr<ClassRegistry> classRegistry,
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
    label << prefix << "_" << labels_[prefix];
    labels_[prefix] += 1;
    return label.str();
  }

  /// \brief Generate a label for a int literal
  ///
  /// \param[in] literal int literal
  /// \return a unique label for the int literal
  std::string generateIntLabel(const int32_t literal) {
    std::stringstream label;
    const char sign = literal >= 0 ? 'P' : 'M';
    label << "Int" << sign << "_" << abs((int64_t)literal);
    ints_.insert(literal);
    return label.str();
  }

  /// \brief Generate a label for a string literal
  ///
  /// \param[in] literal string literal
  /// \return a unique label for the string literal
  std::string generateStringLabel(const std::string &literal) {
    std::stringstream label;
    if (!strings_.count(literal)) {
      strings_[literal] = strings_.size();
    }
    label << "String_" << strings_[literal];
    return label.str();
  }

  /// \brief Return true if a label for the int literal was already generated
  ///
  /// \param[in] literal int literal
  /// \return True if a label for the int literal was already generated
  bool hasIntLabel(const int32_t literal) { return ints_.count(literal) > 0; }

  /// \brief Return true if a label for the string literal was already generated
  ///
  /// \param[in] literal string literal
  /// \return True if a label for the string literal was already generated
  bool hasStringLabel(const std::string &literal) {
    return strings_.count(literal);
  }

  /// \brief Increment stack position by count elements
  ///
  /// \param[in] count size to add to stack position
  void incrementStackPosition(const int32_t count) { stackPosition_ += count; }

  /// \brief Decrement stack position by count elements
  ///
  /// \param[in] count size to subtract from stack position
  void decrementStackPosition(const int32_t count) { stackPosition_ -= count; }

  /// \brief Reset the stack position to zero
  void resetStackPosition() { stackPosition_ = 0; }

  /// \brief Get the stack position
  ///
  /// \return the stack position
  int32_t stackPosition() const { return stackPosition_; }

private:
  int32_t stackPosition_;
  std::unordered_set<int32_t> ints_;
  std::unordered_map<std::string, size_t> labels_;
  std::unordered_map<std::string, size_t> strings_;
};

} // namespace cool

#endif
