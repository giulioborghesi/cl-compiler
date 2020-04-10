#include <cool/analysis/pass.h>
#include <cool/ir/class.h>
#include <cool/ir/expr.h>

namespace cool {

/// ProgramNode
ProgramNode::ProgramNode(std::vector<std::shared_ptr<ClassNode>> *classes)
    : classes_(std::move(*classes)) {}

ProgramNode *
ProgramNode::MakeProgramNode(std::vector<std::shared_ptr<ClassNode>> *classes) {
  return new ProgramNode(classes);
}

/// ClassNode
ClassNode::ClassNode(const std::string &className,
                     const std::string &parentClassName,
                     std::vector<std::shared_ptr<AttributeNode>> *attributes,
                     std::vector<std::shared_ptr<MethodNode>> *methods,
                     const uint32_t lloc, const uint32_t cloc)
    : ParentNode(lloc, cloc), className_(className),
      parentClassName_(parentClassName), attributes_(std::move(*attributes)),
      methods_(std::move(*methods)) {}

ClassNode *ClassNode::MakeClassNode(
    const std::string &className, const std::string &parentClassName,
    std::vector<std::shared_ptr<AttributeNode>> *attributes,
    std::vector<std::shared_ptr<MethodNode>> *methods, const uint32_t lloc,
    const uint32_t cloc) {
  return new ClassNode(className, parentClassName, attributes, methods, lloc,
                       cloc);
}

/// AttributeNode
AttributeNode::AttributeNode(const std::string &id, const std::string &typeName,
                             ExprNode *initExpr, const uint32_t lloc,
                             const uint32_t cloc)
    : ParentNode(lloc, cloc), id_(id), typeName_(typeName),
      initExpr_(initExpr) {}

AttributeNode *AttributeNode::MakeAttributeNode(const std::string &id,
                                                const std::string &typeName,
                                                ExprNode *initExpr,
                                                const uint32_t lloc,
                                                const uint32_t cloc) {
  return new AttributeNode(id, typeName, initExpr, lloc, cloc);
}

/// MethodNode
MethodNode::MethodNode(
    const std::string &id, const std::string &returnTypeName,
    std::vector<std::pair<std::string, std::string>> *arguments,
    const uint32_t lloc, const uint32_t cloc)
    : ParentNode(lloc, cloc), id_(id), returnTypeName_(returnTypeName),
      arguments_(std::move(*arguments)) {}

MethodNode *MethodNode::MakeMethodNode(
    const std::string &id, const std::string &returnTypeName,
    std::vector<std::pair<std::string, std::string>> *arguments,
    const uint32_t lloc, const uint32_t cloc) {
  return new MethodNode(id, returnTypeName, arguments, lloc, cloc);
}

} // namespace cool
