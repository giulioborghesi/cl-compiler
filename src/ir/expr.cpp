#include <cool/analysis/pass.h>
#include <cool/ir/expr.h>

namespace cool {

/// ExprNode
ExprNode::ExprNode(const uint32_t lloc, const uint32_t cloc)
    : Node(lloc, cloc) {}

/// LiteralExprNode
template <typename T>
LiteralExprNode<T>::LiteralExprNode(const T &value, const uint32_t lloc,
                                    const uint32_t cloc)
    : ParentNode(lloc, cloc), value_(value) {}

template <typename T>
LiteralExprNode<T> *
LiteralExprNode<T>::MakeLiteralExprNode(const T &value, const uint32_t lloc,
                                        const uint32_t cloc) {
  return new LiteralExprNode<T>(value, lloc, cloc);
}

template class LiteralExprNode<int32_t>;
template class LiteralExprNode<std::string>;

/// BooleanExprNode
BooleanExprNode::BooleanExprNode(const bool value, const uint32_t lloc,
                                 const uint32_t cloc)
    : ParentNode(lloc, cloc), value_(value) {}

BooleanExprNode *BooleanExprNode::MakeBooleanExprNode(const bool value,
                                                      const uint32_t lloc,
                                                      const uint32_t cloc) {
  return new BooleanExprNode(value, lloc, cloc);
}

/// IdExprNode
IdExprNode::IdExprNode(const std::string &id, const uint32_t lloc,
                       const uint32_t cloc)
    : ParentNode(lloc, cloc), id_(id) {}

IdExprNode *IdExprNode::MakeIdExprNode(const std::string &id,
                                       const uint32_t lloc,
                                       const uint32_t cloc) {
  return new IdExprNode(id, lloc, cloc);
}

/// UnaryExprNode
UnaryExprNode::UnaryExprNode(ExprNode *expr, UnaryOpID opID,
                             const uint32_t lloc, const uint32_t cloc)
    : ParentNode(lloc, cloc), opID_(opID), expr_(expr) {}

UnaryExprNode *UnaryExprNode::MakeUnaryExprNode(ExprNode *expr, UnaryOpID opID,
                                                const uint32_t lloc,
                                                const uint32_t cloc) {
  return new UnaryExprNode(expr, opID, lloc, cloc);
}

/// BinaryExprNode
BinaryExprNode::BinaryExprNode(ExprNode *lhs, ExprNode *rhs,
                               const uint32_t lloc, const uint32_t cloc)
    : ParentNode(lloc, cloc), lhs_(lhs), rhs_(rhs) {}

BinaryExprNode *BinaryExprNode::MakeBinaryExprNode(ExprNode *lhs, ExprNode *rhs,
                                                   const uint32_t lloc,
                                                   const uint32_t cloc) {
  return new BinaryExprNode(lhs, rhs, lloc, cloc);
}

/// IfExprNode
IfExprNode::IfExprNode(ExprNode *ifExpr, ExprNode *thenExpr, ExprNode *elseExpr,
                       const uint32_t lloc, const uint32_t cloc)
    : ParentNode(lloc, cloc), ifExpr_(ifExpr), thenExpr_(thenExpr),
      elseExpr_(elseExpr) {}

IfExprNode *IfExprNode::MakeIfExprNode(ExprNode *ifExpr, ExprNode *thenExpr,
                                       ExprNode *elseExpr, const uint32_t lloc,
                                       const uint32_t cloc) {
  return new IfExprNode(ifExpr, thenExpr, elseExpr, lloc, cloc);
}

/// WhileExprNode
WhileExprNode::WhileExprNode(ExprNode *loopCond, ExprNode *loopBody,
                             const uint32_t lloc, const uint32_t cloc)
    : ParentNode(lloc, cloc), loopCond_(loopCond), loopBody_(loopBody) {}

WhileExprNode *WhileExprNode::MakeWhileExprNode(ExprNode *loopCond,
                                                ExprNode *loopBody,
                                                const uint32_t lloc,
                                                const uint32_t cloc) {
  return new WhileExprNode(loopCond, loopBody, lloc, cloc);
}

/// AssignmentExprNode
AssignmentExprNode::AssignmentExprNode(IdExprNode *id, ExprNode *value,
                                       const uint32_t lloc, const uint32_t cloc)
    : ParentNode(lloc, cloc), id_(id), value_(value) {}

AssignmentExprNode *AssignmentExprNode::MakeAssignmentExprNode(
    IdExprNode *id, ExprNode *value, const uint32_t lloc, const uint32_t cloc) {
  return new AssignmentExprNode(id, value, lloc, cloc);
}

/// BlockExprNode
BlockExprNode::BlockExprNode(std::vector<std::shared_ptr<ExprNode>> *exprs,
                             const uint32_t lloc, const uint32_t cloc)
    : ParentNode(lloc, cloc), exprs_(std::move(*exprs)) {}

BlockExprNode *
BlockExprNode::MakeBlockExprNode(std::vector<std::shared_ptr<ExprNode>> *exprs,
                                 const uint32_t lloc, const uint32_t cloc) {
  return new BlockExprNode(exprs, lloc, cloc);
}

NewExprNode::NewExprNode(const std::string &typeName, const uint32_t lloc,
                         const uint32_t cloc)
    : ParentNode(lloc, cloc), typeName_(typeName) {}

NewExprNode *NewExprNode::MakeNewExprNode(const std::string &typeName,
                                          const uint32_t lloc,
                                          const uint32_t cloc) {
  return new NewExprNode(typeName, lloc, cloc);
}

/// CaseExpr
/*CaseExpr::CaseExpr(Expr *expr, std::vector<std::shared_ptr<Expr>> *ids,
                   std::vector<std::shared_ptr<Expr>> *cases)
    : Expr(), expr_(expr), ids_(std::move(*ids)), cases_(std::move(*cases)) {}

Expr *CaseExpr::MakeCaseExpr(Expr *expr,
                             std::vector<std::shared_ptr<Expr>> *ids,
                             std::vector<std::shared_ptr<Expr>> *cases) {
  return new CaseExpr(expr, ids, cases);
}

Expr *CaseExpr::expr() const { return expr_.get(); }
const std::vector<std::shared_ptr<Expr>> &CaseExpr::ids() const { return ids_; }
const std::vector<std::shared_ptr<Expr>> &CaseExpr::cases() const {
  return cases_;
}*/

} // namespace cool
