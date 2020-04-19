#ifndef COOL_IR_FWD_H
#define COOL_IR_FWD_H

#include <memory>

namespace cool {

/// Base node class
class Node;

/// Program-related nodes
class AttributeNode;
class ClassNode;
class GenericAttributeNode;
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
class CaseBindingNode;
class LetBindingNode;

/// Shared pointer aliases
using AttributeNodePtr = std::shared_ptr<AttributeNode>;
using ClassNodePtr = std::shared_ptr<ClassNode>;
using GenericAttributeNodePtr = std::shared_ptr<GenericAttributeNode>;
using MethodNodePtr = std::shared_ptr<MethodNode>;
using ProgramNodePtr = std::shared_ptr<ProgramNode>;

using AssignmentExprNodePtr = std::shared_ptr<AssignmentExprNode>;
using BlockExprNodePtr = std::shared_ptr<BlockExprNode>;
using BooleanExprNodePtr = std::shared_ptr<BooleanExprNode>;
using CaseExprNodePtr = std::shared_ptr<CaseExprNode>;
using DispatchExprNodePtr = std::shared_ptr<DispatchExprNode>;
using ExprNodePtr = std::shared_ptr<ExprNode>;
using IdExprNodePtr = std::shared_ptr<IdExprNode>;
using IfExprNodePtr = std::shared_ptr<IfExprNode>;
using LetExprNodePtr = std::shared_ptr<LetExprNode>;
using NewExprNodePtr = std::shared_ptr<NewExprNode>;
using StaticDispatchExprNodePtr = std::shared_ptr<StaticDispatchExprNode>;
using UnaryExprNodePtr = std::shared_ptr<UnaryExprNode>;
using WhileExprNodePtr = std::shared_ptr<WhileExprNode>;

using CaseBindingNodePtr = std::shared_ptr<CaseBindingNode>;
using LetBindingNodePtr = std::shared_ptr<LetBindingNode>;

} // namespace cool

#endif
