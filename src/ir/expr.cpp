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
IdExprNode::IdExprNode(const std::string &idName, const uint32_t lloc,
                       const uint32_t cloc)
    : ParentNode(lloc, cloc), idName_(idName) {}

IdExprNode *IdExprNode::MakeIdExprNode(const std::string &idName,
                                       const uint32_t lloc,
                                       const uint32_t cloc) {
  return new IdExprNode(idName, lloc, cloc);
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
template <typename OperatorT>
BinaryExprNode<OperatorT>::BinaryExprNode(ExprNode *lhs, ExprNode *rhs,
                                          OperatorT opID, const uint32_t lloc,
                                          const uint32_t cloc)
    : ParentNode(lloc, cloc), opID_(opID), lhs_(lhs), rhs_(rhs) {}

template <typename OperatorT>
BinaryExprNode<OperatorT> *BinaryExprNode<OperatorT>::MakeBinaryExprNode(
    ExprNode *lhs, ExprNode *rhs, OperatorT opID, const uint32_t lloc,
    const uint32_t cloc) {
  return new BinaryExprNode(lhs, rhs, opID, lloc, cloc);
}

template class BinaryExprNode<ArithmeticOpID>;

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

/// LetBindingNode
LetBindingNode::LetBindingNode(IdExprNode *id, ExprNode *expr,
                               const ExprType &idType, const uint32_t lloc,
                               const uint32_t cloc)
    : ParentNode(lloc, cloc), id_(id), expr_(expr), idType_(idType) {}

LetBindingNode *LetBindingNode::MakeLetBindingNode(IdExprNode *id,
                                                   ExprNode *expr,
                                                   const ExprType &idType,
                                                   const uint32_t lloc,
                                                   const uint32_t cloc) {
  return new LetBindingNode(id, expr, idType, lloc, cloc);
}

/// LetExprNode
LetExprNode::LetExprNode(std::vector<std::shared_ptr<LetBindingNode>> *bindings,
                         ExprNode *expr, const uint32_t lloc,
                         const uint32_t cloc)
    : ParentNode(lloc, cloc), bindings_(std::move(*bindings)), expr_(expr) {}

LetExprNode *LetExprNode::MakeLetExprNode(
    std::vector<std::shared_ptr<LetBindingNode>> *bindings, ExprNode *expr,
    const uint32_t lloc, const uint32_t cloc) {
  return new LetExprNode(bindings, expr, lloc, cloc);
}

/// CaseNode
CaseNode::CaseNode(IdExprNode *id, ExprNode *expr, ExprType idType,
                   const uint32_t lloc, const uint32_t cloc)
    : ParentNode(lloc, cloc), id_(id), expr_(expr), idType_(idType) {}

CaseNode *CaseNode::MakeCaseNode(IdExprNode *id, ExprNode *expr,
                                 ExprType idType, const uint32_t lloc,
                                 const uint32_t cloc) {
  return new CaseNode(id, expr, idType, lloc, cloc);
}

/// CaseExprNode
CaseExprNode::CaseExprNode(std::vector<std::shared_ptr<CaseNode>> *cases,
                           ExprNode *expr, const uint32_t lloc,
                           const uint32_t cloc)
    : ParentNode(lloc, cloc), cases_(std::move(*cases)), expr_(expr) {}

CaseExprNode *
CaseExprNode::MakeCaseExprNode(std::vector<std::shared_ptr<CaseNode>> *cases,
                               ExprNode *expr, const uint32_t lloc,
                               const uint32_t cloc) {
  return new CaseExprNode(cases, expr, lloc, cloc);
}

/// DispatchExprNode
DispatchExprNode::DispatchExprNode(const std::string &funcName, ExprNode *expr,
                                   std::vector<std::shared_ptr<ExprNode>> *args,
                                   const uint32_t lloc, const uint32_t cloc)
    : ParentNode(lloc, cloc), funcName_(funcName), expr_(expr),
      args_(std::move(*args)) {}

DispatchExprNode *DispatchExprNode::MakeDispatchExprNode(
    const std::string &funcName, ExprNode *expr,
    std::vector<std::shared_ptr<ExprNode>> *args, const uint32_t lloc,
    const uint32_t cloc) {
  return new DispatchExprNode(funcName, expr, args, lloc, cloc);
}

/// StaticDispatchExprNode
StaticDispatchExprNode::StaticDispatchExprNode(
    const std::string &funcName, const std::string &dispatchClass,
    ExprNode *expr, std::vector<std::shared_ptr<ExprNode>> *args,
    const uint32_t lloc, const uint32_t cloc)
    : ParentNode(lloc, cloc), funcName_(funcName),
      dispatchClass_(dispatchClass), expr_(expr), args_(std::move(*args)) {}

StaticDispatchExprNode *StaticDispatchExprNode::MakeStaticDispatchExprNode(
    const std::string &funcName, const std::string &dispatchClass,
    ExprNode *expr, std::vector<std::shared_ptr<ExprNode>> *args,
    const uint32_t lloc, const uint32_t cloc) {
  return new StaticDispatchExprNode(funcName, dispatchClass, expr, args, lloc,
                                    cloc);
}

} // namespace cool
