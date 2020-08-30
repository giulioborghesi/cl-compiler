#include <cool/codegen/codegen_base.h>
#include <cool/codegen/codegen_context.h>
#include <cool/ir/class.h>
#include <cool/ir/expr.h>

namespace cool {

Status CodegenBasePass::codegen(CodegenContext *context, AttributeNode *node,
                                std::ostream *ios) {
  if (node->initExpr()) {
    node->initExpr()->generateCode(context, this, ios);
  }
  return Status::Ok();
}

Status CodegenBasePass::codegen(CodegenContext *context, ClassNode *node,
                                std::ostream *ios) {
  for (auto attributeNode : node->attributes()) {
    attributeNode->generateCode(context, this, ios);
  }

  for (auto methodNode : node->methods()) {
    methodNode->generateCode(context, this, ios);
  }

  return Status::Ok();
}

Status CodegenBasePass::codegen(CodegenContext *context, MethodNode *node,
                                std::ostream *ios) {
  if (node->body()) {
    node->body()->generateCode(context, this, ios);
  }

  for (auto argumentNode : node->arguments()) {
    argumentNode->generateCode(context, this, ios);
  }

  return Status::Ok();
}

Status CodegenBasePass::codegen(CodegenContext *context, ProgramNode *node,
                                std::ostream *ios) {
  for (auto classNode : node->classes()) {
    classNode->generateCode(context, this, ios);
  }
  return Status::Ok();
}

Status CodegenBasePass::codegen(CodegenContext *context,
                                AssignmentExprNode *node, std::ostream *ios) {
  node->rhsExpr()->generateCode(context, this, ios);
  return Status::Ok();
}

Status CodegenBasePass::codegen(CodegenContext *context,
                                BinaryExprNode<ArithmeticOpID> *node,
                                std::ostream *ios) {
  node->lhsExpr()->generateCode(context, this, ios);
  node->rhsExpr()->generateCode(context, this, ios);
  return Status::Ok();
}

Status CodegenBasePass::codegen(CodegenContext *context,
                                BinaryExprNode<ComparisonOpID> *node,
                                std::ostream *ios) {
  node->lhsExpr()->generateCode(context, this, ios);
  node->rhsExpr()->generateCode(context, this, ios);
  return Status::Ok();
}

Status CodegenBasePass::codegen(CodegenContext *context, BlockExprNode *node,
                                std::ostream *ios) {
  for (auto exprNode : node->exprs()) {
    exprNode->generateCode(context, this, ios);
  }
  return Status::Ok();
}

Status CodegenBasePass::codegen(CodegenContext *context, CaseBindingNode *node,
                                std::ostream *ios) {
  node->expr()->generateCode(context, this, ios);
  return Status::Ok();
}

Status CodegenBasePass::codegen(CodegenContext *context, CaseExprNode *node,
                                std::ostream *ios) {
  for (auto exprNode : node->cases()) {
    exprNode->generateCode(context, this, ios);
  }
  return Status::Ok();
}

Status CodegenBasePass::codegen(CodegenContext *context, DispatchExprNode *node,
                                std::ostream *ios) {
  for (auto paramNode : node->params()) {
    paramNode->generateCode(context, this, ios);
  }

  if (node->hasExpr()) {
    node->expr()->generateCode(context, this, ios);
  }
  return Status::Ok();
}

Status CodegenBasePass::codegen(CodegenContext *context, IfExprNode *node,
                                std::ostream *ios) {
  node->ifExpr()->generateCode(context, this, ios);
  node->thenExpr()->generateCode(context, this, ios);
  node->elseExpr()->generateCode(context, this, ios);
  return Status::Ok();
}

Status CodegenBasePass::codegen(CodegenContext *context, LetBindingNode *node,
                                std::ostream *ios) {
  if (node->hasExpr()) {
    node->expr()->generateCode(context, this, ios);
  }
  return Status::Ok();
}

Status CodegenBasePass::codegen(CodegenContext *context, LetExprNode *node,
                                std::ostream *ios) {
  for (auto bindingNode : node->bindings()) {
    bindingNode->generateCode(context, this, ios);
  }

  node->expr()->generateCode(context, this, ios);
  return Status::Ok();
}

Status CodegenBasePass::codegen(CodegenContext *context,
                                StaticDispatchExprNode *node,
                                std::ostream *ios) {
  for (auto paramNode : node->params()) {
    paramNode->generateCode(context, this, ios);
  }

  node->expr()->generateCode(context, this, ios);
  return Status::Ok();
}

Status CodegenBasePass::codegen(CodegenContext *context, UnaryExprNode *node,
                                std::ostream *ios) {
  node->expr()->generateCode(context, this, ios);
  return Status::Ok();
}

Status CodegenBasePass::codegen(CodegenContext *context, WhileExprNode *node,
                                std::ostream *ios) {
  node->loopCond()->generateCode(context, this, ios);
  node->loopBody()->generateCode(context, this, ios);
  return Status::Ok();
}

} // namespace cool
