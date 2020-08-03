#include <cool/analysis/pass.h>
#include <cool/ir/expr.h>

namespace cool {

/// ExprNode
ExprNode::ExprNode(const uint32_t lloc, const uint32_t cloc)
    : Node(lloc, cloc) {}

/// AssignmentExprNode
AssignmentExprNode::AssignmentExprNode(const std::string &id,
                                       ExprNodePtr rhsExpr, const uint32_t lloc,
                                       const uint32_t cloc)
    : ParentNode(lloc, cloc), id_(id), rhsExpr_(rhsExpr) {}

AssignmentExprNodePtr AssignmentExprNode::MakeAssignmentExprNode(
    const std::string &id, ExprNodePtr rhsExpr, const uint32_t lloc,
    const uint32_t cloc) {
  return AssignmentExprNodePtr(new AssignmentExprNode(id, rhsExpr, lloc, cloc));
}

/// BlockExprNode
BlockExprNode::BlockExprNode(std::vector<ExprNodePtr> exprs,
                             const uint32_t lloc, const uint32_t cloc)
    : ParentNode(lloc, cloc), exprs_(std::move(exprs)) {}

BlockExprNodePtr
BlockExprNode::MakeBlockExprNode(std::vector<ExprNodePtr> exprs,
                                 const uint32_t lloc, const uint32_t cloc) {
  return BlockExprNodePtr(new BlockExprNode(std::move(exprs), lloc, cloc));
}

/// CaseBindingNode
CaseBindingNode::CaseBindingNode(const std::string &id,
                                 const std::string &typeName, ExprNodePtr expr,
                                 const uint32_t lloc, const uint32_t cloc)
    : ParentNode(lloc, cloc), id_(id), typeName_(typeName), bindingLabel_(),
      expr_(expr) {}

CaseBindingNodePtr CaseBindingNode::MakeCaseBindingNode(
    const std::string &id, const std::string &typeName, ExprNodePtr expr,
    const uint32_t lloc, const uint32_t cloc) {
  return CaseBindingNodePtr(
      new CaseBindingNode(id, typeName, expr, lloc, cloc));
}

/// CaseExprNode
CaseExprNode::CaseExprNode(std::vector<CaseBindingNodePtr> cases,
                           ExprNodePtr expr, const uint32_t lloc,
                           const uint32_t cloc)
    : ParentNode(lloc, cloc), cases_(std::move(cases)), expr_(expr) {}

CaseExprNodePtr
CaseExprNode::MakeCaseExprNode(std::vector<CaseBindingNodePtr> cases,
                               ExprNodePtr expr, const uint32_t lloc,
                               const uint32_t cloc) {
  return CaseExprNodePtr(new CaseExprNode(std::move(cases), expr, lloc, cloc));
}

/// LiteralExprNode
template <typename T>
LiteralExprNode<T>::LiteralExprNode(const T &value, const uint32_t lloc,
                                    const uint32_t cloc)
    : ParentNode(lloc, cloc), value_(value) {}

template <typename T>
std::shared_ptr<LiteralExprNode<T>>
LiteralExprNode<T>::MakeLiteralExprNode(const T &value, const uint32_t lloc,
                                        const uint32_t cloc) {
  return std::shared_ptr<LiteralExprNode<T>>(
      new LiteralExprNode<T>(value, lloc, cloc));
}

template class LiteralExprNode<int32_t>;
template class LiteralExprNode<std::string>;

/// BooleanExprNode
BooleanExprNode::BooleanExprNode(const bool value, const uint32_t lloc,
                                 const uint32_t cloc)
    : ParentNode(lloc, cloc), value_(value) {}

BooleanExprNodePtr BooleanExprNode::MakeBooleanExprNode(const bool value,
                                                        const uint32_t lloc,
                                                        const uint32_t cloc) {
  return BooleanExprNodePtr(new BooleanExprNode(value, lloc, cloc));
}

/// IdExprNode
IdExprNode::IdExprNode(const std::string &id, const uint32_t lloc,
                       const uint32_t cloc)
    : ParentNode(lloc, cloc), id_(id) {}

IdExprNodePtr IdExprNode::MakeIdExprNode(const std::string &id,
                                         const uint32_t lloc,
                                         const uint32_t cloc) {
  return IdExprNodePtr(new IdExprNode(id, lloc, cloc));
}

/// UnaryExprNode
UnaryExprNode::UnaryExprNode(ExprNodePtr expr, UnaryOpID opID,
                             const uint32_t lloc, const uint32_t cloc)
    : ParentNode(lloc, cloc), opID_(opID), expr_(expr) {}

UnaryExprNodePtr UnaryExprNode::MakeUnaryExprNode(ExprNodePtr expr,
                                                  UnaryOpID opID,
                                                  const uint32_t lloc,
                                                  const uint32_t cloc) {
  return UnaryExprNodePtr(new UnaryExprNode(expr, opID, lloc, cloc));
}

/// BinaryExprNode
template <typename OperatorT>
BinaryExprNode<OperatorT>::BinaryExprNode(ExprNodePtr lhsExpr,
                                          ExprNodePtr rhsExpr, OperatorT opID,
                                          const uint32_t lloc,
                                          const uint32_t cloc)
    : ParentNode(lloc, cloc), opID_(opID), lhsExpr_(lhsExpr),
      rhsExpr_(rhsExpr) {}

template <typename OperatorT>
std::shared_ptr<BinaryExprNode<OperatorT>>
BinaryExprNode<OperatorT>::MakeBinaryExprNode(ExprNodePtr lhsExpr,
                                              ExprNodePtr rhsExpr,
                                              OperatorT opID,
                                              const uint32_t lloc,
                                              const uint32_t cloc) {
  return std::shared_ptr<BinaryExprNode<OperatorT>>(
      new BinaryExprNode(lhsExpr, rhsExpr, opID, lloc, cloc));
}

template class BinaryExprNode<ArithmeticOpID>;
template class BinaryExprNode<ComparisonOpID>;

/// IfExprNode
IfExprNode::IfExprNode(ExprNodePtr ifExpr, ExprNodePtr thenExpr,
                       ExprNodePtr elseExpr, const uint32_t lloc,
                       const uint32_t cloc)
    : ParentNode(lloc, cloc), ifExpr_(ifExpr), thenExpr_(thenExpr),
      elseExpr_(elseExpr) {}

IfExprNodePtr IfExprNode::MakeIfExprNode(ExprNodePtr ifExpr,
                                         ExprNodePtr thenExpr,
                                         ExprNodePtr elseExpr,
                                         const uint32_t lloc,
                                         const uint32_t cloc) {
  return IfExprNodePtr(new IfExprNode(ifExpr, thenExpr, elseExpr, lloc, cloc));
}

/// WhileExprNode
WhileExprNode::WhileExprNode(ExprNodePtr loopCond, ExprNodePtr loopBody,
                             const uint32_t lloc, const uint32_t cloc)
    : ParentNode(lloc, cloc), loopCond_(loopCond), loopBody_(loopBody) {}

WhileExprNodePtr WhileExprNode::MakeWhileExprNode(ExprNodePtr loopCond,
                                                  ExprNodePtr loopBody,
                                                  const uint32_t lloc,
                                                  const uint32_t cloc) {
  return WhileExprNodePtr(new WhileExprNode(loopCond, loopBody, lloc, cloc));
}

/// NewExprNode
NewExprNode::NewExprNode(const std::string &typeName, const uint32_t lloc,
                         const uint32_t cloc)
    : ParentNode(lloc, cloc), typeName_(typeName) {}

NewExprNodePtr NewExprNode::MakeNewExprNode(const std::string &typeName,
                                            const uint32_t lloc,
                                            const uint32_t cloc) {
  return NewExprNodePtr(new NewExprNode(typeName, lloc, cloc));
}

/// LetBindingNode
LetBindingNode::LetBindingNode(const std::string &id,
                               const std::string &typeName, ExprNodePtr expr,
                               const uint32_t lloc, const uint32_t cloc)
    : ParentNode(lloc, cloc), id_(id), typeName_(typeName), expr_(expr) {}

LetBindingNodePtr LetBindingNode::MakeLetBindingNode(
    const std::string &id, const std::string &typeName, ExprNodePtr expr,
    const uint32_t lloc, const uint32_t cloc) {
  return LetBindingNodePtr(new LetBindingNode(id, typeName, expr, lloc, cloc));
}

/// LetExprNode
LetExprNode::LetExprNode(std::vector<LetBindingNodePtr> bindings,
                         ExprNodePtr expr, const uint32_t lloc,
                         const uint32_t cloc)
    : ParentNode(lloc, cloc), bindings_(std::move(bindings)), expr_(expr) {}

LetExprNodePtr
LetExprNode::MakeLetExprNode(std::vector<LetBindingNodePtr> bindings,
                             ExprNodePtr expr, const uint32_t lloc,
                             const uint32_t cloc) {
  return LetExprNodePtr(new LetExprNode(std::move(bindings), expr, lloc, cloc));
}

/// DispatchExprNode
DispatchExprNode::DispatchExprNode(const std::string &methodName,
                                   ExprNodePtr expr,
                                   std::vector<ExprNodePtr> params,
                                   const uint32_t lloc, const uint32_t cloc)
    : ParentNode(lloc, cloc), methodName_(methodName), expr_(expr),
      params_(std::move(params)) {}

DispatchExprNodePtr DispatchExprNode::MakeDispatchExprNode(
    const std::string &methodName, ExprNodePtr expr,
    std::vector<ExprNodePtr> params, const uint32_t lloc, const uint32_t cloc) {
  return DispatchExprNodePtr(
      new DispatchExprNode(methodName, expr, std::move(params), lloc, cloc));
}

/// StaticDispatchExprNode
StaticDispatchExprNode::StaticDispatchExprNode(const std::string &methodName,
                                               const std::string &callerClass,
                                               ExprNodePtr expr,
                                               std::vector<ExprNodePtr> params,
                                               const uint32_t lloc,
                                               const uint32_t cloc)
    : ParentNode(lloc, cloc), methodName_(methodName),
      callerClass_(callerClass), expr_(expr), params_(std::move(params)) {}

StaticDispatchExprNodePtr StaticDispatchExprNode::MakeStaticDispatchExprNode(
    const std::string &methodName, const std::string &callerClass,
    ExprNodePtr expr, std::vector<ExprNodePtr> params, const uint32_t lloc,
    const uint32_t cloc) {
  return StaticDispatchExprNodePtr(new StaticDispatchExprNode(
      methodName, callerClass, expr, std::move(params), lloc, cloc));
}

} // namespace cool
