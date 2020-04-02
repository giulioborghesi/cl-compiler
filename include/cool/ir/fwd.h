#ifndef COOL_IR_FWD_H
#define COOL_IR_FWD_H

namespace cool {

/// Base node class
class Node;

/// Program-related nodes
class AttributeNode;
class ClassNode;
class MethodNode;
class ProgramNode;

/// Expressions node
class AssignmentExprNode;
template <typename T> class BinaryExprNode;
class BlockExprNode;
class BooleanExprNode;
class ExprNode;
class IdExprNode;
class IfExprNode;
class LetBindingExprNode;
class LetExprNode;
template <typename T> class LiteralExprNode;
class NewExprNode;
class UnaryExprNode;
class WhileExprNode;

} // namespace cool

#endif
