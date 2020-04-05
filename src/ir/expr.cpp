#include <cool/analysis/pass.h>
#include <cool/ir/expr.h>

namespace cool {

/// ExprNode
ExprNode::ExprNode(const uint32_t lloc, const uint32_t cloc)
    : Node(lloc, cloc) {}

/// AssignmentExprNode
AssignmentExprNode::AssignmentExprNode(const std::string &id, ExprNode *rhsExpr,
                                       const uint32_t lloc, const uint32_t cloc)
    : ParentNode(lloc, cloc), id_(id), rhsExpr_(rhsExpr) {}

AssignmentExprNode *AssignmentExprNode::MakeAssignmentExprNode(
    const std::string &id, ExprNode *rhsExpr, const uint32_t lloc,
    const uint32_t cloc) {
  return new AssignmentExprNode(id, rhsExpr, lloc, cloc);
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

/// CaseNode
CaseNode::CaseNode(const std::string &id, const std::string &typeName,
                   ExprNode *expr, const uint32_t lloc, const uint32_t cloc)
    : ParentNode(lloc, cloc), id_(id), typeName_(typeName), expr_(expr) {}

CaseNode *CaseNode::MakeCaseNode(const std::string &id,
                                 const std::string &typeName, ExprNode *expr,
                                 const uint32_t lloc, const uint32_t cloc) {
  return new CaseNode(id, typeName, expr, lloc, cloc);
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
template <typename OperatorT>
BinaryExprNode<OperatorT>::BinaryExprNode(ExprNode *lhsExpr, ExprNode *rhsExpr,
                                          OperatorT opID, const uint32_t lloc,
                                          const uint32_t cloc)
    : ParentNode(lloc, cloc), opID_(opID), lhsExpr_(lhsExpr),
      rhsExpr_(rhsExpr) {}

template <typename OperatorT>
BinaryExprNode<OperatorT> *BinaryExprNode<OperatorT>::MakeBinaryExprNode(
    ExprNode *lhsExpr, ExprNode *rhsExpr, OperatorT opID, const uint32_t lloc,
    const uint32_t cloc) {
  return new BinaryExprNode(lhsExpr, rhsExpr, opID, lloc, cloc);
}

template class BinaryExprNode<ArithmeticOpID>;
template class BinaryExprNode<ComparisonOpID>;

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

/// NewExprNode
NewExprNode::NewExprNode(const std::string &typeName, const uint32_t lloc,
                         const uint32_t cloc)
    : ParentNode(lloc, cloc), typeName_(typeName) {}

NewExprNode *NewExprNode::MakeNewExprNode(const std::string &typeName,
                                          const uint32_t lloc,
                                          const uint32_t cloc) {
  return new NewExprNode(typeName, lloc, cloc);
}

/// LetBindingNode
LetBindingNode::LetBindingNode(const std::string &id,
                               const std::string &typeName, ExprNode *expr,
                               const uint32_t lloc, const uint32_t cloc)
    : ParentNode(lloc, cloc), id_(id), typeName_(typeName), expr_(expr) {}

LetBindingNode *LetBindingNode::MakeLetBindingNode(const std::string &id,
                                                   const std::string &typeName,
                                                   ExprNode *expr,
                                                   const uint32_t lloc,
                                                   const uint32_t cloc) {
  return new LetBindingNode(id, typeName, expr, lloc, cloc);
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

/// DispatchExprNode
DispatchExprNode::DispatchExprNode(
    const std::string &methodName, ExprNode *expr,
    std::vector<std::shared_ptr<ExprNode>> *params, const uint32_t lloc,
    const uint32_t cloc)
    : ParentNode(lloc, cloc), methodName_(methodName), expr_(expr),
      params_(std::move(*params)) {}

DispatchExprNode *DispatchExprNode::MakeDispatchExprNode(
    const std::string &methodName, ExprNode *expr,
    std::vector<std::shared_ptr<ExprNode>> *params, const uint32_t lloc,
    const uint32_t cloc) {
  return new DispatchExprNode(methodName, expr, params, lloc, cloc);
}

/// StaticDispatchExprNode
StaticDispatchExprNode::StaticDispatchExprNode(
    const std::string &methodName, const std::string &callerClass,
    ExprNode *expr, std::vector<std::shared_ptr<ExprNode>> *params,
    const uint32_t lloc, const uint32_t cloc)
    : ParentNode(lloc, cloc), methodName_(methodName),
      callerClass_(callerClass), expr_(expr), params_(std::move(*params)) {}

StaticDispatchExprNode *StaticDispatchExprNode::MakeStaticDispatchExprNode(
    const std::string &methodName, const std::string &callerClass,
    ExprNode *expr, std::vector<std::shared_ptr<ExprNode>> *params,
    const uint32_t lloc, const uint32_t cloc) {
  return new StaticDispatchExprNode(methodName, callerClass, expr, params, lloc,
                                    cloc);
}

} // namespace cool
