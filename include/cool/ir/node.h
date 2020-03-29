#ifndef COOL_IR_NODE_H
#define COOL_IR_NODE_H

#include <cool/ir/common.h>

#include <cstdlib>

namespace cool {

/// Forward declarations
class Context;
class Pass;

/// Base class for a node in the abstract syntax tree
class Node {

public:
  Node() = delete;
  Node(const uint32_t lloc, const uint32_t cloc) : lloc_(lloc), cloc_(cloc) {}

  virtual ~Node() = default;

  /// Get the location in the current line of the program text where the node is
  /// defined
  ///
  /// \return the location in the current line of the program text where the
  /// node is defined
  uint32_t getLineLoc() const { return lloc_; }

  /// Get the location in the current line of the program text where the node is
  /// defined
  ///
  /// \return the location in the current line of the program text where the
  /// node is defined
  uint32_t getCharLoc() const { return cloc_; }

private:
  uint32_t lloc_ = 0;
  uint32_t cloc_ = 0;
};

} // namespace cool

#endif
