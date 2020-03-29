#include <cstdlib>
#include <string>

namespace cool {

/// Forward declarations
class Context;

template <typename T> class LiteralExprNode;
template <bool T> class BooleanExprNode;

template <size_t UnaryExprT> class UnaryExprNode;
template <size_t BinaryExprT> class BinaryExprNode;

class IfExprNode;
class WhileExprNode;
class AssignmentExprNode;
class BlockExprNode;
class NewExprNode;
class NewIdExprNode;
class LetExprNode;

class Visitor {
  Visitor() = default;
  virtual ~Visitor() = default;
};

class TypeCheckVisitor : public Visitor {

public:
  TypeCheckVisitor() = default;
  ~TypeCheckVisitor() final override = default;

  template <typename T>
  bool visit(Context *context, LiteralExprNode<T> *node) const;

  template <bool V>
  bool visit(Context *context, BooleanExprNode<T> *node) const;

  bool visit(Context *context, IdExprNode *node) const;

  template <size_t UnaryExprT>
  bool visit(Context *context, UnaryExprNode<UnaryExprT> *node) const;

  template <size_t BinaryExprT>
  bool visit(Context *context, BinaryExprNode<BinaryExprT> *node) const;

  bool visit(Context *context, IfExprNode *node) const;

  bool visit(Context *context, WhileExprNode *node) const;

  bool visit(Context *context, AssignmentExprNode *node) const;

  bool visit(Context *context, BlockExprNode *node) const;

  bool visit(Context *context, NewExprNode *node) const;

  bool visit(Context *context, NewIdExprNode *node) const;

  bool visit(Context *context, LetExprNode *node) const;
};

template <bool Value>
bool TypeCheckVisitor<Value>::visit(Context *context,
                                    BooleanExprNode<Value> *node) const {
  node->setType(context->symbolTable()["Bool"]);
  return true;
}

template <>
bool TypeCheckVisitor<int32_t>::visit(Context *context,
                                      LiteralExprNode<int32_t> *node) const {
  node->setType(context->symbolTable()["Int"]);
  return true;
}

template <>
bool TypeCheckVisitor<std::string>::visit(
    Context *context, LiteralExprNode<std::string> *node) const {
  node->setType(context->symbolTable()["String"]);
  return true;
}

} // namespace cool