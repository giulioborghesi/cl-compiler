#ifndef COOL_ANALYSIS_PASS_H
#define COOL_ANALYSIS_PASS_H

#include <cool/core/status.h>
#include <cool/ir/common.h>
#include <cool/ir/fwd.h>

#include <cstdlib>
#include <string>

namespace cool {

// Forward declaration
class AnalysisContext;

class Pass {

public:
  Pass() = default;
  virtual ~Pass() = default;

  /// Program, class and attributes nodes
  virtual Status visit(AnalysisContext *context, AttributeNode *node) {
    return Status::Ok();
  }

  virtual Status visit(AnalysisContext *context, ClassNode *node) {
    return Status::Ok();
  }

  virtual Status visit(AnalysisContext *context, FormalNode *node) {
    return Status::Ok();
  }

  virtual Status visit(AnalysisContext *context, MethodNode *node) {
    return Status::Ok();
  }

  virtual Status visit(AnalysisContext *context, ProgramNode *node) {
    return Status::Ok();
  }

  /// Expressions nodes
  virtual Status visit(AnalysisContext *context, AssignmentExprNode *node) {
    return Status::Ok();
  }

  virtual Status visit(AnalysisContext *context,
                       BinaryExprNode<ArithmeticOpID> *node) {
    return Status::Ok();
  }

  virtual Status visit(AnalysisContext *context,
                       BinaryExprNode<ComparisonOpID> *node) {
    return Status::Ok();
  }

  virtual Status visit(AnalysisContext *context, BlockExprNode *node) {
    return Status::Ok();
  }

  virtual Status visit(AnalysisContext *context, BooleanExprNode *node) {
    return Status::Ok();
  }

  virtual Status visit(AnalysisContext *context, CaseBindingNode *node) {
    return Status::Ok();
  }

  virtual Status visit(AnalysisContext *context, CaseExprNode *node) {
    return Status::Ok();
  }

  virtual Status visit(AnalysisContext *context, DispatchExprNode *node) {
    return Status::Ok();
  }

  virtual Status visit(AnalysisContext *context, ExprNode *node) {
    return Status::Ok();
  }

  virtual Status visit(AnalysisContext *context, IdExprNode *node) {
    return Status::Ok();
  }

  virtual Status visit(AnalysisContext *context, IfExprNode *node) {
    return Status::Ok();
  }

  virtual Status visit(AnalysisContext *context, LetBindingNode *node) {
    return Status::Ok();
  }

  virtual Status visit(AnalysisContext *context, LetExprNode *node) {
    return Status::Ok();
  }

  virtual Status visit(AnalysisContext *context,
                       LiteralExprNode<int32_t> *node) {
    return Status::Ok();
  }

  virtual Status visit(AnalysisContext *context,
                       LiteralExprNode<std::string> *node) {
    return Status::Ok();
  }

  virtual Status visit(AnalysisContext *context, NewExprNode *node) {
    return Status::Ok();
  }

  virtual Status visit(AnalysisContext *context, StaticDispatchExprNode *node) {
    return Status::Ok();
  }

  virtual Status visit(AnalysisContext *context, UnaryExprNode *node) {
    return Status::Ok();
  }

  virtual Status visit(AnalysisContext *context, WhileExprNode *node) {
    return Status::Ok();
  }
};

} // namespace cool

#endif
