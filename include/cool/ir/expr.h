#ifndef COOL_IR_EXPR_H
#define COOL_IR_EXPR_H

#include <cool/ir/common.h>
#include <cool/ir/node.h>
#include <cool/ir/visitable.h>

#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

namespace cool {

/// Base class for a node representing an expression in the AST
class ExprNode : public Node {

public:
  ExprNode() = delete;
  ExprNode(const uint32_t lloc, const uint32_t cloc);
  ~ExprNode() override = default;

  /// Return the expression type
  const ExprType &type() const { return type_; }

  /// Set the expression type to the specified type
  ///
  /// \param[in] type expression type
  void setType(const ExprType &type) { type_ = type; }

private:
  ExprType type_;
};

/// Base class for a terminal node in the AST representing a literal
template <typename T>
class LiteralExprNode : public Visitable<ExprNode, LiteralExprNode<T>> {

  using ParentNode = Visitable<ExprNode, LiteralExprNode<T>>;

public:
  LiteralExprNode() = delete;
  ~LiteralExprNode() final override = default;

  /// Factory method to create a node for a literal expression node
  ///
  /// \param[in] value literal expression value
  /// \param[in] lloc line location
  /// \param[in] cloc character location
  /// \return a pointer to the new literal expression node
  static LiteralExprNode *
  MakeLiteralExprNode(const T &value, const uint32_t lloc, const uint32_t cloc);

  /// Get the value stored by the literal node
  const T &value() const { return value_; };

private:
  LiteralExprNode(const T &value, const uint32_t lloc, const uint32_t cloc);
  const T value_;
};

/// Base class for a terminal node in the AST representing a boolean
class BooleanExprNode : public Visitable<ExprNode, BooleanExprNode> {

  using ParentNode = Visitable<ExprNode, BooleanExprNode>;

public:
  BooleanExprNode() = delete;
  ~BooleanExprNode() final override = default;

  /// Factory method to create a node for a boolean expression node
  ///
  /// \param[in] value value of boolean expression
  /// \param[in] lloc line location
  /// \param[in] cloc character location
  /// \return a pointer to the new literal expression node
  static BooleanExprNode *MakeBooleanExprNode(const bool value,
                                              const uint32_t lloc,
                                              const uint32_t cloc);

  /// Get the value of the boolean expression
  ///
  /// \return the value of the boolean expression
  bool value() const { return value_; }

private:
  BooleanExprNode(const bool value, const uint32_t lloc, const uint32_t cloc);

  const bool value_;
};

/// Base class for a terminal node in the AST representing an identifier
class IdExprNode : public Visitable<ExprNode, IdExprNode> {

  using ParentNode = Visitable<ExprNode, IdExprNode>;

public:
  IdExprNode() = delete;
  ~IdExprNode() final override = default;

  /// Factory method to create a node for an id expression
  ///
  /// \param[in] idName identifier name
  /// \param[in] lloc line location
  /// \param[in] cloc character location
  /// \return a pointer to the new id expression node
  static IdExprNode *MakeIdExprNode(const std::string &idName,
                                    const uint32_t lloc, const uint32_t cloc);

  /// Get the identifier name
  ///
  /// \return the identifier name
  const std::string &idName() const { return idName_; }

private:
  IdExprNode(const std::string &idName, const uint32_t lloc,
             const uint32_t cloc);

  const std::string idName_;
};

/// Base class for a node representing a unary expression in the AST
class UnaryExprNode : public Visitable<ExprNode, UnaryExprNode> {

  using ParentNode = Visitable<ExprNode, UnaryExprNode>;

public:
  UnaryExprNode() = delete;
  ~UnaryExprNode() final override = default;

  /// Factory method to create a node representing a unary expression
  ///
  /// \param[in] expr operand of unary operator
  /// \param[in] opID unary operation identifier
  /// \param[in] lloc line location
  /// \param[in] cloc character location
  /// \return a pointer to the new unary expression node
  static UnaryExprNode *MakeUnaryExprNode(ExprNode *expr, UnaryOpID opID,
                                          const uint32_t lloc,
                                          const uint32_t cloc);

  /// Get the node of the subexpression representing the operand
  ///
  /// \return a shared pointer to the subexpression node
  std::shared_ptr<ExprNode> expr() const { return expr_; }

  /// Get the operation ID
  ///
  /// \return the operation ID
  UnaryOpID opID() const { return opID_; }

private:
  UnaryExprNode(ExprNode *expr, UnaryOpID opID, const uint32_t lloc,
                const uint32_t cloc);

  UnaryOpID opID_;
  const std::shared_ptr<ExprNode> expr_;
};

/// Base class for a node representing a binary expression in the AST
template <typename OperatorT>
class BinaryExprNode : public Visitable<ExprNode, BinaryExprNode<OperatorT>> {

  using ParentNode = Visitable<ExprNode, BinaryExprNode<OperatorT>>;

public:
  BinaryExprNode() = delete;
  ~BinaryExprNode() override = default;

  /// Factory method to create a node representing a binary expression
  ///
  /// \param[in] lhs left operand of binary operator
  /// \param[in] rhs right operand of binary operator
  /// \param[in] opID operator ID
  /// \param[in] lloc line location
  /// \param[in] cloc character location
  /// \return a pointer to the new binary expression node
  static BinaryExprNode *MakeBinaryExprNode(ExprNode *lhs, ExprNode *rhs,
                                            const OperatorT opID,
                                            const uint32_t lloc,
                                            const uint32_t cloc);

  /// Get the node of the subexpression representing the left operand
  ///
  /// \return a shared pointer to the left subexpression node
  std::shared_ptr<ExprNode> lhs() const { return lhs_; }

  /// Get the operator ID
  ///
  /// \return the operator ID
  OperatorT opID() const { return opID_; }

  /// Get the node of the subexpression representing the right operand
  ///
  /// \return a shared pointer to the right subexpression node
  std::shared_ptr<ExprNode> rhs() const { return rhs_; }

private:
  BinaryExprNode(ExprNode *lhs, ExprNode *rhs, const OperatorT opID,
                 const uint32_t lloc, const uint32_t cloc);

  const OperatorT opID_;
  const std::shared_ptr<ExprNode> lhs_;
  const std::shared_ptr<ExprNode> rhs_;
};

/// Class for a node representing an if expression
class IfExprNode : public Visitable<ExprNode, IfExprNode> {

  using ParentNode = Visitable<ExprNode, IfExprNode>;

public:
  IfExprNode() = delete;
  ~IfExprNode() final override = default;

  /// Factory method to create a node for an if expression
  ///
  /// \param[in] ifExpr if expression
  /// \param[in] thenExpr then expression
  /// \param[in] elseExpr else expression
  /// \param[in] lloc line location
  /// \param[in] cloc character location
  /// \return a pointer to the new if expression node
  static IfExprNode *MakeIfExprNode(ExprNode *ifExpr, ExprNode *thenExpr,
                                    ExprNode *elseExpr, const uint32_t lloc,
                                    const uint32_t cloc);

  /// Get the node of the subexpression representing the if condition
  ///
  /// \return a shared pointer to the if subexpression node
  std::shared_ptr<ExprNode> ifExpr() const { return ifExpr_; }

  /// Get the node of the subexpression representing the then expression
  ///
  /// \return a shared pointer to the then subexpression node
  std::shared_ptr<ExprNode> thenExpr() const { return thenExpr_; }

  /// Get the node of the subexpression representing the else expression
  ///
  /// \return a shared pointer to the else subexpression node
  std::shared_ptr<ExprNode> elseExpr() const { return elseExpr_; }

private:
  IfExprNode(ExprNode *ifExpr, ExprNode *thenExpr, ExprNode *elseExpr,
             const uint32_t lloc, const uint32_t cloc);

  const std::shared_ptr<ExprNode> ifExpr_;
  const std::shared_ptr<ExprNode> thenExpr_;
  const std::shared_ptr<ExprNode> elseExpr_;
};

/// Class for a node representing a while expression
class WhileExprNode : public Visitable<ExprNode, WhileExprNode> {

  using ParentNode = Visitable<ExprNode, WhileExprNode>;

public:
  WhileExprNode() = delete;
  ~WhileExprNode() final override = default;

  /// Factory method to create a node for a while expression
  ///
  /// \param[in] loopCond expression for the loop condition
  /// \param[in] loopBody expression for the loop body
  /// \param[in] lloc line location
  /// \param[in] cloc character location
  /// \return a pointer to the new while expression node
  static WhileExprNode *MakeWhileExprNode(ExprNode *loopCond,
                                          ExprNode *loopBody,
                                          const uint32_t lloc,
                                          const uint32_t cloc);

  /// Get the node of the subexpression representing the loop condition
  ///
  /// \return a shared pointer to the node representing the loop condition
  std::shared_ptr<ExprNode> loopCond() const { return loopCond_; }

  /// Get the node of the subexpression representing the loop body
  ///
  /// \return a shared pointer to the node representing the loop body
  std::shared_ptr<ExprNode> loopBody() const { return loopBody_; }

private:
  WhileExprNode(ExprNode *loopCond, ExprNode *loopBody, const uint32_t lloc,
                const uint32_t cloc);

  const std::shared_ptr<ExprNode> loopCond_;
  const std::shared_ptr<ExprNode> loopBody_;
};

/// Class for a node representing an assignment expression
class AssignmentExprNode : public Visitable<ExprNode, AssignmentExprNode> {

  using ParentNode = Visitable<ExprNode, AssignmentExprNode>;

public:
  AssignmentExprNode() = delete;
  ~AssignmentExprNode() final override = default;

  /// Factory method to create a node for an assignment expression
  ///
  /// \param[in] id identifier expression
  /// \param[in] value value assigned to identifier
  /// \param[in] lloc line location
  /// \param[in] cloc character location
  /// \return a pointer to the new assignment expression node
  static AssignmentExprNode *MakeAssignmentExprNode(IdExprNode *id,
                                                    ExprNode *value,
                                                    const uint32_t lloc,
                                                    const uint32_t cloc);

  /// Get the node of the subexpression representing the identifier
  ///
  /// \return a shared pointer to the node representing the identifier
  /// expression
  std::shared_ptr<IdExprNode> id() const { return id_; }

  /// Get the node of the subexpression representing the expression on the
  /// right hand side of the assignment expression
  ///
  /// \return a shared pointer to the node representing the expression on the
  /// right hand side of the assignment expression
  std::shared_ptr<ExprNode> value() const { return value_; }

private:
  AssignmentExprNode(IdExprNode *id, ExprNode *value, const uint32_t lloc,
                     const uint32_t cloc);

  const std::shared_ptr<IdExprNode> id_;
  const std::shared_ptr<ExprNode> value_;
};

/// Class for a node representing a block expression
class BlockExprNode : public Visitable<ExprNode, BlockExprNode> {

  using ParentNode = Visitable<ExprNode, BlockExprNode>;

public:
  BlockExprNode() = delete;
  ~BlockExprNode() final override = default;

  /// Factory method to create a node for a block expression
  ///
  /// \param[in] exprs expressions in the block
  /// \param[in] lloc line location
  /// \param[in] cloc character location
  /// \return a pointer to the new block expression node
  static BlockExprNode *
  MakeBlockExprNode(std::vector<std::shared_ptr<ExprNode>> *exprs,
                    const uint32_t lloc, const uint32_t cloc);

  /// Get the nodes of the subexpressions in the block
  const std::vector<std::shared_ptr<ExprNode>> &exprs() const { return exprs_; }

private:
  BlockExprNode(std::vector<std::shared_ptr<ExprNode>> *exprs,
                const uint32_t lloc, const uint32_t cloc);

  const std::vector<std::shared_ptr<ExprNode>> exprs_;
};

/// Class for a node representing a new expression
class NewExprNode : public Visitable<ExprNode, NewExprNode> {

  using ParentNode = Visitable<ExprNode, NewExprNode>;

public:
  NewExprNode() = delete;
  ~NewExprNode() final override = default;

  /// Factory method to create a node for a new expression
  ///
  /// \param[in] typeName type of new object
  /// \param[in] lloc line location
  /// \param[in] cloc character location
  /// \return a pointer to the new block expression node
  static NewExprNode *MakeNewExprNode(const std::string &typeName,
                                      const uint32_t lloc, const uint32_t cloc);

  /// Get the type name of the new object
  ///
  /// \return the type name of the new object
  const std::string &typeName() const { return typeName_; }

private:
  NewExprNode(const std::string &typeName, const uint32_t lloc,
              const uint32_t cloc);

  std::string typeName_;
};

/// Class for a node representing a let binding
class LetBindingNode : public Visitable<Node, LetBindingNode> {

  using ParentNode = Visitable<Node, LetBindingNode>;

public:
  LetBindingNode() = delete;
  ~LetBindingNode() final override = default;

  /// Factory method to create a node for a new identifier declaration node
  ///
  /// \param[in] id identifier expression
  /// \param[in] expr identifier initialization expression
  /// \param[in] idType identifier static type
  /// \param[in] lloc line location
  /// \param[in] cloc character location
  /// \return a pointer to the new block expression node
  static LetBindingNode *MakeLetBindingNode(IdExprNode *id, ExprNode *expr,
                                            const ExprType &idType,
                                            const uint32_t lloc,
                                            const uint32_t cloc);

  /// Return whether the identifier has an initialization expression
  bool hasExpr() const { return expr_ != nullptr; }

  /// Return a pointer to the id expression
  ///
  /// \return a pointer to the id expression
  std::shared_ptr<IdExprNode> id() const { return id_; }

  /// Return a pointer to the identifier initialization expression
  ///
  /// \return a pointer to the identifier initialization expression
  std::shared_ptr<ExprNode> expr() const { return expr_; }

  /// Return the static type of the identifier
  const ExprType &idType() const { return idType_; }

private:
  LetBindingNode(IdExprNode *id, ExprNode *expr, const ExprType &idType,
                 const uint32_t lloc, const uint32_t cloc);

  const std::shared_ptr<IdExprNode> id_;
  const std::shared_ptr<ExprNode> expr_;
  const ExprType idType_;
};

/// Class for a node representing a let expression
class LetExprNode : public Visitable<ExprNode, LetExprNode> {

  using ParentNode = Visitable<ExprNode, LetExprNode>;

public:
  LetExprNode() = delete;
  ~LetExprNode() final override = default;

  /// Factory method to create a node for a let expression node
  ///
  /// \param[in] bindings list of pointers to the let bindings
  /// \param[in] expr pointer to let expression
  /// \param[in] lloc line location
  /// \param[in] cloc character location
  /// \return a pointer to the new let expression node
  static LetExprNode *
  MakeLetExprNode(std::vector<std::shared_ptr<LetBindingNode>> *bindings,
                  ExprNode *expr, const uint32_t lloc, const uint32_t cloc);

  /// Return a list of pointers to the let bindings
  ///
  /// \return a vector of shared pointers to the let bindings nodes
  const std::vector<std::shared_ptr<LetBindingNode>> &bindings() const {
    return bindings_;
  }

  /// Return a pointer to the expression in the let construct
  ///
  /// \return a shared pointer to the node for the expression in the let
  /// construct
  std::shared_ptr<ExprNode> expr() const { return expr_; }

private:
  LetExprNode(std::vector<std::shared_ptr<LetBindingNode>> *bindings,
              ExprNode *expr, const uint32_t lloc, const uint32_t cloc);

  const std::vector<std::shared_ptr<LetBindingNode>> bindings_;
  const std::shared_ptr<ExprNode> expr_;
};

/// Class for a node representing a case in a case expression
class CaseNode : public Visitable<Node, CaseNode> {

  using ParentNode = Visitable<Node, CaseNode>;

public:
  CaseNode() = delete;
  ~CaseNode() final override = default;

  /// Factory method to create a node for a case node
  ///
  /// \param[in] id pointer to identifier
  /// \param[in] expr pointer to expression
  /// \param[in] idType identifier type
  /// \param[in] lloc line location
  /// \param[in] cloc character location
  /// \return a pointer to the new case node
  static CaseNode *MakeCaseNode(IdExprNode *id, ExprNode *expr, ExprType idType,
                                const uint32_t lloc, const uint32_t cloc);

  /// Return a pointer to the identifier node
  ///
  /// \return a shared pointer to the identifier node
  std::shared_ptr<IdExprNode> id() const { return id_; }

  /// Return the formal identifier type
  ///
  /// \return the identifier type
  ExprType idType() const { return idType_; }

  /// Return a pointer to the expression node
  ///
  /// \return a shared pointer to the expression node
  std::shared_ptr<ExprNode> expr() const { return expr_; }

private:
  CaseNode(IdExprNode *id, ExprNode *expr, ExprType idType, const uint32_t lloc,
           const uint32_t cloc);

  const std::shared_ptr<IdExprNode> id_;
  const std::shared_ptr<ExprNode> expr_;
  const ExprType idType_;
};

/// Class for a node representing a case expression
class CaseExprNode : public Visitable<ExprNode, CaseExprNode> {

  using ParentNode = Visitable<ExprNode, CaseExprNode>;

public:
  CaseExprNode() = delete;
  ~CaseExprNode() final override = default;

  /// Factory method to create a node for a case expression node
  ///
  /// \param[in] cases cases in case expression
  /// \param[in] expr pointer to case expression
  /// \param[in] lloc line location
  /// \param[in] cloc character location
  /// \return a pointer to the new case expression node
  static CaseExprNode *
  MakeCaseExprNode(std::vector<std::shared_ptr<CaseNode>> *cases,
                   ExprNode *expr, const uint32_t lloc, const uint32_t cloc);

  /// Return a list of pointers to the case nodes
  ///
  /// \return a vector of shared pointers to the case nodes
  const std::vector<std::shared_ptr<CaseNode>> &cases() const { return cases_; }

  /// Return a pointer to the expression in the case statement
  std::shared_ptr<ExprNode> expr() const { return expr_; }

private:
  CaseExprNode(std::vector<std::shared_ptr<CaseNode>> *cases, ExprNode *expr,
               const uint32_t lloc, const uint32_t cloc);

  const std::vector<std::shared_ptr<CaseNode>> cases_;
  const std::shared_ptr<ExprNode> expr_;
};

/// Class for a node representing a dispatch expression
class DispatchExprNode : public Visitable<ExprNode, DispatchExprNode> {

  using ParentNode = Visitable<ExprNode, DispatchExprNode>;

public:
  DispatchExprNode() = delete;
  ~DispatchExprNode() final override = default;

  /// Factory method to create a node for a dispatch expression
  ///
  /// \param[in] funcName function name
  /// \param[in] expr expression on which function is called
  /// \param[in] args function arguments
  /// \param[in] lloc line location
  /// \param[in] cloc character location
  /// \return a pointer to the newly created dispatch expression node
  static DispatchExprNode *
  MakeDispatchExprNode(const std::string &funcName, ExprNode *expr,
                       std::vector<std::shared_ptr<ExprNode>> *args,
                       const uint32_t lloc, const uint32_t cloc);

  /// Return a list of pointers to the function arguments nodes
  ///
  /// \return a vector of shared pointers to the function arguments nodes
  const std::vector<std::shared_ptr<ExprNode>> &args() const { return args_; }

  /// Return the number of function arguments
  ///
  /// \return the number of function arguments
  const uint32_t argSize() const { return args_.size(); }

  /// Return the expression node
  ///
  /// \return a shared pointer to the expression node
  std::shared_ptr<ExprNode> expr() const { return expr_; }

  /// Return the function name
  ///
  /// \return the function name
  const std::string &funcName() const { return funcName_; }

  /// Return whether the expression node is present or not
  ///
  /// \return true if the expression exists, false otherwise
  bool hasExpr() const { return expr_ != nullptr; }

private:
  DispatchExprNode(const std::string &funcName, ExprNode *expr,
                   std::vector<std::shared_ptr<ExprNode>> *args,
                   const uint32_t lloc, const uint32_t cloc);

  std::string funcName_;
  std::shared_ptr<ExprNode> expr_;
  std::vector<std::shared_ptr<ExprNode>> args_;
};

/// Class for a node representing a static dispatch expression
class StaticDispatchExprNode
    : public Visitable<ExprNode, StaticDispatchExprNode> {

  using ParentNode = Visitable<ExprNode, StaticDispatchExprNode>;

public:
  StaticDispatchExprNode() = delete;
  ~StaticDispatchExprNode() final override = default;

  /// Factory method to create a node for a static dispatch expression
  ///
  /// \param[in] funcName function name
  /// \param[in] dispatchClass dispatch class name
  /// \param[in] expr expression on which function is called
  /// \param[in] args function arguments
  /// \param[in] lloc line location
  /// \param[in] cloc character location
  /// \return a pointer to the newly created static dispatch expression node
  static StaticDispatchExprNode *
  MakeStaticDispatchExprNode(const std::string &funcName,
                             const std::string &dispatchClass, ExprNode *expr,
                             std::vector<std::shared_ptr<ExprNode>> *args,
                             const uint32_t lloc, const uint32_t cloc);

  /// Return a list of pointers to the function arguments nodes
  ///
  /// \return a vector of shared pointers to the function arguments nodes
  const std::vector<std::shared_ptr<ExprNode>> &args() const { return args_; }

  /// Return the number of function arguments
  ///
  /// \return the number of function arguments
  const uint32_t argSize() const { return args_.size(); }

  /// Return the parent class name on which the function should be called
  ///
  /// \return the parent class name on which the function should be called
  const std::string &dispatchClass() const { return dispatchClass_; }

  /// Return the expression node
  ///
  /// \return a shared pointer to the expression node
  std::shared_ptr<ExprNode> expr() const { return expr_; }

  /// Return the function name
  ///
  /// \return the function name
  const std::string &funcName() const { return funcName_; }

private:
  StaticDispatchExprNode(const std::string &funcName,
                         const std::string &dispatchClass, ExprNode *expr,
                         std::vector<std::shared_ptr<ExprNode>> *args,
                         const uint32_t lloc, const uint32_t cloc);

  std::string funcName_;
  std::string dispatchClass_;
  std::shared_ptr<ExprNode> expr_;
  std::vector<std::shared_ptr<ExprNode>> args_;
};

} // namespace cool

#endif
