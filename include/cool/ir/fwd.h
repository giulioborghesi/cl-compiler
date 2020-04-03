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

/// Expressions nodes
class AssignmentExprNode;
template <typename T> class BinaryExprNode;
class BlockExprNode;
class BooleanExprNode;
class CaseExprNode;
class DispatchExprNode;
class ExprNode;
class IdExprNode;
class IfExprNode;
class LetExprNode;
template <typename T> class LiteralExprNode;
class NewExprNode;
class StaticDispatchExprNode;
class UnaryExprNode;
class WhileExprNode;

/// Untyped expression components nodes
class CaseNode;
class LetBindingNode;

} // namespace cool

#endif
