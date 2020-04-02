#ifndef COOL_ANALYSIS_PASS_H
#define COOL_ANALYSIS_PASS_H

#include <cool/core/status.h>
#include <cool/ir/fwd.h>

#include <cstdlib>
#include <string>

namespace cool {

// Forward declaration
class Context;

class Pass {

public:
  Pass() = default;
  virtual ~Pass() = default;

  virtual Status visit(Context *context, ProgramNode *node) {
    return Status::Ok();
  }

  virtual Status visit(Context *context, ClassNode *node) {
    return Status::Ok();
  }

  virtual Status visit(Context *context, ExprNode *node) {
    return Status::Ok();
  }

  virtual Status visit(Context *context, LiteralExprNode<int32_t> *node) {
    return Status::Ok();
  }

  virtual Status visit(Context *context, LiteralExprNode<std::string> *node) {
    return Status::Ok();
  }

  virtual Status visit(Context *context, BooleanExprNode *node) {
    return Status::Ok();
  }

  virtual Status visit(Context *context, IdExprNode *node) {
    return Status::Ok();
  };

  virtual Status visit(Context *context, UnaryExprNode *node) {
    return Status::Ok();
  }

  virtual Status visit(Context *context, BinaryExprNode *node) {
    return Status::Ok();
  }

  virtual Status visit(Context *context, IfExprNode *node) {
    return Status::Ok();
  }

  virtual Status visit(Context *context, WhileExprNode *node) {
    return Status::Ok();
  }

  virtual Status visit(Context *context, AssignmentExprNode *node) {
    return Status::Ok();
  }

  virtual Status visit(Context *context, BlockExprNode *node) {
    return Status::Ok();
  }

  virtual Status visit(Context *context, NewExprNode *node) {
    return Status::Ok();
  }

  virtual Status visit(Context *context, LetBindingExprNode *node) {
    return Status::Ok();
  }

  virtual Status visit(Context *context, LetExprNode *node) {
    return Status::Ok();
  }
};

} // namespace cool

#endif
