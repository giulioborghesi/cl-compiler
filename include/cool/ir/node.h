#ifndef COOL_IR_NODE_H
#define COOL_IR_NODE_H

#include <cool/core/status.h>
#include <cool/ir/common.h>

#include <cstdlib>

namespace cool {

/// Forward declarations
class Context;
class Pass;
class CodegenPass;

/// Base class for a node in the abstract syntax tree
class Node {

public:
  Node() = delete;
  virtual ~Node() = default;

  /// Get the location in the current line of the program text where the node is
  /// defined
  ///
  /// \return the location in the current line of the program text where the
  /// node is defined
  uint32_t lineLoc() const { return lloc_; }

  /// Get the location in the current line of the program text where the node is
  /// defined
  ///
  /// \return the location in the current line of the program text where the
  /// node is defined
  uint32_t charLoc() const { return cloc_; }

  /// Visit the node and execute the operation associated with the analysis pass
  ///
  /// \param[in] context pass context
  /// \param[in] pass analysis pass
  /// \return Status::Ok() on success, an error message otherwise
  virtual Status visitNode(Context *context, Pass *pass) = 0;

  /// Visit the node and generate code
  ///
  /// \param[in] context codegen context
  /// \param[in] pass codengen pass
  /// \return Status::Ok() on success, an error message otherwise
  virtual Status generateCode(Context *context, CodegenPass *pass) = 0;

protected:
  Node(const uint32_t lloc, const uint32_t cloc) : lloc_(lloc), cloc_(cloc) {}

private:
  uint32_t lloc_ = 0;
  uint32_t cloc_ = 0;
};

} // namespace cool

#endif
