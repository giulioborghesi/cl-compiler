#ifndef COOL_IR_EXPR_H
#define COOL_IR_EXPR_H

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
  ~ExprNode() override = 0;

  /// Return the expression type
  ExprType type() const { return type_; }

  /// Set the expression type to the specified type
  ///
  /// \param[in] type expression type
  void setType(const ExprType &type) { type_ = type; }

private:
  ExprType type_;
};

/// Base class for a terminal node in the AST representing a literal
template <typename T>
class LiteralExprNode : public ExprNode, public Visitable<LiteralExprNode<T>> {

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

using IntExprNode = LiteralExprNode<int32_t>;
using StringExprNode = LiteralExprNode<std::string>;

/// Base class for a terminal node in the AST representing a boolean
class BooleanExprNode : public ExprNode, public Visitable<BooleanExprNode> {

public:
  BooleanExprNode() = delete;
  ~BooleanExprNode() final override = default;

  /// Factory method to create a node for a boolean expression node
  ///
  /// \param[in] lloc line location
  /// \param[in] cloc character location
  /// \return a pointer to the new literal expression node
  static BooleanExprNode *MakeBooleanExprNode(const uint32_t lloc,
                                              const uint32_t cloc);

private:
  BooleanExprNode(const uint32_t lloc, const uint32_t cloc);
};

/// Base class for a terminal node in the AST representing an identifier
class IdExprNode : public ExprNode, public Visitable<IdExprNode> {

public:
  IdExprNode() = delete;
  ~IdExprNode() final override = default;

  /// Factory method to create a node for an id expression
  ///
  /// \param[in] id identifier name
  /// \param[in] lloc line location
  /// \param[in] cloc character location
  /// \return a pointer to the new id expression node
  static IdExprNode *MakeIdExprNode(const std::string &id, const uint32_t lloc,
                                    const uint32_t cloc);

  /// Get the identifier id
  ///
  /// \return the identifier id
  const std::string &id() const { return id_; }

private:
  IdExprNode(const std::string &id, const uint32_t lloc, const uint32_t cloc);

  const std::string id_;
};

/// Base class for a node representing a unary expression in the AST
class UnaryExprNode : public ExprNode, public Visitable<UnaryExprNode> {

public:
  UnaryExprNode() = delete;
  ~UnaryExprNode() final override = default;

  /// Factory method to create a node representing a unary expression
  ///
  /// \param[in] expr operand of unary operator
  /// \param[in] lloc line location
  /// \param[in] cloc character location
  /// \return a pointer to the new unary expression node
  static UnaryExprNode *MakeUnaryExprNode(ExprNode *expr, const uint32_t lloc,
                                          const uint32_t cloc);

  /// Get the node of the subexpression representing the operand
  ///
  /// \return a shared pointer to the subexpression node
  std::shared_ptr<ExprNode> expr() const { return expr_; }

private:
  UnaryExprNode(ExprNode *expr, const uint32_t lloc, const uint32_t cloc);

  const std::shared_ptr<ExprNode> expr_;
};

/// Base class for a node representing a binary expression in the AST
class BinaryExprNode : public ExprNode, public Visitable<BinaryExprNode> {

public:
  BinaryExprNode() = delete;
  ~BinaryExprNode() override = default;

  /// Factory method to create a node representing a binary expression
  ///
  /// \param[in] lhs left operand of binary operator
  /// \param[in] rhs right operand of binary operator
  /// \param[in] lloc line location
  /// \param[in] cloc character location
  /// \return a pointer to the new binary expression node
  static BinaryExprNode *MakeBinaryExprNode(ExprNode *lhs, ExprNode *rhs,
                                            const uint32_t lloc,
                                            const uint32_t cloc);

  /// Get the node of the subexpression representing the left operand
  ///
  /// \return a shared pointer to the left subexpression node
  std::shared_ptr<ExprNode> lhs() const { return lhs_; }

  /// Get the node of the subexpression representing the right operand
  ///
  /// \return a shared pointer to the right subexpression node
  std::shared_ptr<ExprNode> rhs() const { return rhs_; }

private:
  BinaryExprNode(ExprNode *lhs, ExprNode *rhs, const uint32_t lloc,
                 const uint32_t cloc);

  const std::shared_ptr<ExprNode> lhs_;
  const std::shared_ptr<ExprNode> rhs_;
};

/// Class for a node representing an if expression
class IfExprNode : public ExprNode, public Visitable<IfExprNode> {

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
class WhileExprNode : public ExprNode, public Visitable<WhileExprNode> {

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
class AssignmentExprNode : public ExprNode,
                           public Visitable<AssignmentExprNode> {

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

  /// Get the node of the subexpression representing the expression on the right
  /// hand side of the assignment expression
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
class BlockExprNode : public ExprNode, public Visitable<BlockExprNode> {

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
  const std::vector<std::shared_ptr<ExprNode>> &exprs() const;

private:
  BlockExprNode(std::vector<std::shared_ptr<ExprNode>> *exprs,
                const uint32_t lloc, const uint32_t cloc);

  const std::vector<std::shared_ptr<ExprNode>> exprs_;
};

/// Class for a node representing a new expression
class NewExprNode : public ExprNode, public Visitable<NewExprNode> {

public:
  NewExprNode() = delete;
  ~NewExprNode() final override = default;

  /// Factory method to create a node for a new expression
  ///
  /// \param[in] exprType type of new object
  /// \param[in] lloc line location
  /// \param[in] cloc character location
  /// \return a pointer to the new block expression node
  static NewExprNode *MakeNewExprNode(const ExprType &exprType,
                                      const uint32_t lloc, const uint32_t cloc);

private:
  NewExprNode(const ExprType &exprType, const uint32_t lloc,
              const uint32_t cloc);
};

/// Class for a node representing an identifier declaration
class NewIdExprNode : public ExprNode, public Visitable<NewIdExprNode> {

public:
  NewIdExprNode() = delete;
  ~NewIdExprNode() final override = default;

  /// Factory method to create a node for a new identifier declaration node
  ///
  /// \param[in] id identifier expression
  /// \param[in] expr identifier initialization expression
  /// \param[in] idType identifier static type
  /// \param[in] lloc line location
  /// \param[in] cloc character location
  /// \return a pointer to the new block expression node
  static NewIdExprNode *MakeNewIdExprNode(IdExprNode *id, ExprNode *expr,
                                          const ExprType &idType,
                                          const uint32_t lloc,
                                          const uint32_t cloc);

  /// Return a pointer to the id expression
  std::shared_ptr<IdExprNode> id() const { return id_; }

  /// Return a pointer to the expression used to initialize the id value
  std::shared_ptr<ExprNode> expr() const { return expr_; }

  /// Return the static type of the identifier
  const ExprType &idType() const { return idType_; }

private:
  NewIdExprNode(IdExprNode *id, ExprNode *expr, const ExprType &idType,
                const uint32_t lloc, const uint32_t cloc);

  std::shared_ptr<IdExprNode> id_;
  std::shared_ptr<ExprNode> expr_;
  const ExprType idType_;
};

/// Class for a node representing a let expression
class LetExprNode : public ExprNode, public Visitable<LetExprNode> {

public:
  LetExprNode() = delete;
  ~LetExprNode() final override = default;

  /// Factory method to create a node for a let expression node
  ///
  /// \param[in] newIds new identifier expressions
  /// \param[in] expr let expression
  /// \param[in] lloc line location
  /// \param[in] cloc character location
  /// \return a pointer to the new block expression node
  static LetExprNode *
  MakeLetExprNode(std::vector<std::shared_ptr<NewIdExprNode>> *newIds,
                  ExprNode *expr, const uint32_t lloc, const uint32_t cloc);

  /// Return the pointers to the new Id expressions
  ///
  /// \return a vector of shared pointers to the let bindings nodes
  const std::vector<std::shared_ptr<NewIdExprNode>> &newIds() const {
    return newIds_;
  }

  /// Return a pointer to the expression in the let construct
  ///
  /// \return a shared pointer to the node for the expression in the let
  /// construct
  std::shared_ptr<ExprNode> expr() const { return expr_; }

private:
  LetExprNode(std::vector<std::shared_ptr<NewIdExprNode>> *newIds,
              ExprNode *expr, const uint32_t lloc, const uint32_t cloc);

  std::vector<std::shared_ptr<NewIdExprNode>> newIds_;
  std::shared_ptr<ExprNode> expr_;
};

} // namespace cool

#endif
