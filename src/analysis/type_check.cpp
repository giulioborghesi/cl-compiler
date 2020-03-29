#include <cool/passes/type_check.h>

#include <iostream>

namespace cool {

bool TypeCheckVisitor::visit(Context *context, BlockExprNode *node) const {
  /// Typecheck expressions in block
  bool successBlock = true;
  for (auto expr : node->exprs()) {
    const bool successExpr = expr->get()->visit(context, expr->get());
    successBlock = successBlock && successExpr;
  }

  /// Assign type to block expression if no error occurred in nested expressions
  if (successBlock) {
    node->type() = node->exprs().back()->type();
  }

  return successBlock;
}

bool TypeCheckVisitor::visit(Context *context, IdExprNode *node) const {
  /// Identifier must be in scope
  if (context->symbolTable().find(node->id()) == 0) {
    std::cerr << "Error: identifier " << node->id() << " is not in scope"
              << std::endl;
    return false;
  }

  /// Assign type to expression and return
  node->type() = context->symbolTable()[node->id()];
  return true;
}

bool TypeCheckVisitor::visit(Context *context, AssignmentExprNode *node) const {
  /// Traverse the children nodes
  const bool successI = node->id()->visit(context, this);
  const bool successV = node->value()->visit(context, this);
  bool success = successI && successV;

  /// Assign type to expression if possible
  if (success) {
    if (!context->isChildOf(node->value()->type(), node->id()->type())) {
      std::cerr << "Error: type " << node->value()->type()
                << " is not a subtype of " << node->id()->type() << std::endl;
      success = false;
    } else {
      node->setType(node->value()->type());
    }
  }

  return success;
}

} // namespace cool

} // namespace cool
