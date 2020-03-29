#ifndef COOL_ANALYSIS_PASS_H
#define COOL_ANALYSIS_PASS_H

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

  virtual bool visit(Context *context, ProgramNode *node) { return true; }

  virtual bool visit(Context *context, ClassNode *node) { return true; }

  virtual bool visit(Context *context, LiteralExprNode<int32_t> *node) {
    return true;
  }

  virtual bool visit(Context *context, LiteralExprNode<std::string> *node) {
    return true;
  }

  virtual bool visit(Context *context, BooleanExprNode *node) { return true; }

  virtual bool visit(Context *context, IdExprNode *node) { return true; };

  virtual bool visit(Context *context, UnaryExprNode *node) { return true; }

  virtual bool visit(Context *context, BinaryExprNode *node) { return true; }

  virtual bool visit(Context *context, IfExprNode *node) { return true; }

  virtual bool visit(Context *context, WhileExprNode *node) { return true; }

  virtual bool visit(Context *context, AssignmentExprNode *node) {
    return true;
  }

  virtual bool visit(Context *context, BlockExprNode *node) { return true; }

  virtual bool visit(Context *context, NewExprNode *node) { return true; }

  virtual bool visit(Context *context, NewIdExprNode *node) { return true; }

  virtual bool visit(Context *context, LetExprNode *node) { return true; }
};

} // namespace cool

#endif
