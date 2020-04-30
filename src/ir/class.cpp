#include <cool/analysis/pass.h>
#include <cool/ir/class.h>
#include <cool/ir/expr.h>

#include <string>
#include <unordered_map>

namespace cool {

namespace {

/// \brief Helper function to sort the classes in topological sort if possible
///
/// \param[in] classNodes unsorted class nodes
/// \return the class nodes in sorted order, if possible
std::vector<ClassNodePtr>
SortClasses(const std::vector<ClassNodePtr> &classNodes) {
  std::vector<ClassNodePtr> classStack;
  std::unordered_map<std::string, std::vector<ClassNodePtr>> inheritanceTree;
  for (auto classNode : classNodes) {
    if (classNode->hasParentClass()) {
      inheritanceTree[classNode->parentClassName()].push_back(classNode);
    } else {
      classStack.push_back(classNode);
    }
  }

  std::vector<ClassNodePtr> sortedClassNodes;
  while (classStack.size()) {
    auto topClass = classStack.back();
    classStack.pop_back();
    sortedClassNodes.push_back(topClass);

    if (inheritanceTree.count(topClass->className())) {
      for (auto childClass : inheritanceTree[topClass->className()]) {
        classStack.push_back(childClass);
      }
    }
  }

  if (sortedClassNodes.size() == classNodes.size()) {
    return sortedClassNodes;
  }
  return classNodes;
}

} // namespace

/// ProgramNode
ProgramNode::ProgramNode(std::vector<ClassNodePtr> classes)
    : classes_(std::move(classes)) {}

ProgramNodePtr ProgramNode::MakeProgramNode(std::vector<ClassNodePtr> classes) {
  //  InstallBuiltInClasses(classes);
  auto sortedClasses = SortClasses(classes);
  return ProgramNodePtr(new ProgramNode(std::move(sortedClasses)));
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
MethodNode::MethodNode(const std::string &id, const std::string &returnTypeName,
                       std::vector<FormalNodePtr> arguments,
                       const uint32_t lloc, const uint32_t cloc)
    : ParentNode(lloc, cloc), id_(id), returnTypeName_(returnTypeName),
      arguments_(std::move(arguments)) {}

MethodNodePtr MethodNode::MakeMethodNode(const std::string &id,
                                         const std::string &returnTypeName,
                                         std::vector<FormalNodePtr> arguments,
                                         const uint32_t lloc,
                                         const uint32_t cloc) {
  return MethodNodePtr(
      new MethodNode(id, returnTypeName, std::move(arguments), lloc, cloc));
}

/// FormalNode
FormalNode::FormalNode(const std::string &id, const std::string &typeName,
                       const uint32_t lloc, const uint32_t cloc)
    : ParentNode(lloc, cloc), id_(id), typeName_(typeName) {}

FormalNodePtr FormalNode::MakeFormalNode(const std::string &id,
                                         const std::string &typeName,
                                         const uint32_t lloc,
                                         const uint32_t cloc) {
  return FormalNodePtr(new FormalNode(id, typeName, lloc, cloc));
}

} // namespace cool
