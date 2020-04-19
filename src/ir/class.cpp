#include <cool/analysis/pass.h>
#include <cool/ir/class.h>
#include <cool/ir/expr.h>

namespace cool {

/// ProgramNode
ProgramNode::ProgramNode(std::vector<ClassNodePtr> classes)
    : classes_(std::move(classes)) {}

ProgramNodePtr ProgramNode::MakeProgramNode(std::vector<ClassNodePtr> classes) {
  return std::shared_ptr<ProgramNode>(new ProgramNode(std::move(classes)));
}

/// ClassNode
ClassNode::ClassNode(const std::string &className,
                     const std::string &parentClassName,
                     std::vector<GenericAttributeNodePtr> attributes,
                     const uint32_t lloc, const uint32_t cloc)
    : ParentNode(lloc, cloc), className_(className),
      parentClassName_(parentClassName), attributes_(std::move(attributes)) {}

ClassNodePtr
ClassNode::MakeClassNode(const std::string &className,
                         const std::string &parentClassName,
                         std::vector<GenericAttributeNodePtr> attributes,
                         const uint32_t lloc, const uint32_t cloc) {
  return ClassNodePtr(new ClassNode(className, parentClassName,
                                    std::move(attributes), lloc, cloc));
}

/// AttributeNode
AttributeNode::AttributeNode(const std::string &id, const std::string &typeName,
                             ExprNodePtr initExpr, const uint32_t lloc,
                             const uint32_t cloc)
    : ParentNode(lloc, cloc), id_(id), typeName_(typeName),
      initExpr_(initExpr) {}

AttributeNodePtr AttributeNode::MakeAttributeNode(const std::string &id,
                                                  const std::string &typeName,
                                                  ExprNodePtr initExpr,
                                                  const uint32_t lloc,
                                                  const uint32_t cloc) {
  return AttributeNodePtr(
      new AttributeNode(id, typeName, initExpr, lloc, cloc));
}

/// MethodNode
MethodNode::MethodNode(
    const std::string &id, const std::string &returnTypeName,
    std::vector<std::pair<std::string, std::string>> arguments,
    const uint32_t lloc, const uint32_t cloc)
    : ParentNode(lloc, cloc), id_(id), returnTypeName_(returnTypeName),
      arguments_(std::move(arguments)) {}

MethodNodePtr MethodNode::MakeMethodNode(
    const std::string &id, const std::string &returnTypeName,
    std::vector<std::pair<std::string, std::string>> arguments,
    const uint32_t lloc, const uint32_t cloc) {
  return MethodNodePtr(
      new MethodNode(id, returnTypeName, std::move(arguments), lloc, cloc));
}

} // namespace cool
