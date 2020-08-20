#include <cool/analysis/pass.h>
#include <cool/ir/class.h>
#include <cool/ir/expr.h>

#include <algorithm>
#include <queue>
#include <string>
#include <unordered_map>

namespace cool {

/// ProgramNode
ProgramNode::ProgramNode(std::vector<ClassNodePtr> classes)
    : ParentNode(0, 0), classes_(std::move(classes)) {}

ProgramNodePtr ProgramNode::MakeProgramNode(std::vector<ClassNodePtr> classes) {
  return ProgramNodePtr(new ProgramNode(std::move(classes)));
}

Status ProgramNode::sortClasses() {
  std::unordered_map<ClassNodePtr, uint32_t> classOrder;
  std::unordered_map<std::string, std::vector<ClassNodePtr>> edges;

  /// Construct adjacency list representation of class tree
  for (auto classNode : classes_) {
    if (classNode->hasParentClass()) {
      classOrder[classNode] += 1;
      edges[classNode->parentClassName()].push_back(classNode);
    }
  }

  /// Initialize nodes with no parent
  std::queue<ClassNodePtr> frontier;
  for (auto classNode : classes_) {
    if (!classOrder.count(classNode)) {
      frontier.push(classNode);
    }
  }

  /// Order classes
  std::vector<ClassNodePtr> sortedClasses;
  while (frontier.size()) {
    auto rootClass = frontier.front();
    frontier.pop();

    sortedClasses.push_back(rootClass);
    if (edges.count(rootClass->className())) {
      for (auto childClass : edges[rootClass->className()]) {
        classOrder[childClass] -= 1;
        if (classOrder[childClass] == 0) {
          frontier.push(childClass);
        }
      }
    }
  }

  /// Return an error message if a cyclic class definition is detected
  if (sortedClasses.size() < classes_.size()) {
    return GenericError("Error. Cyclic classes definition detected");
  }

  /// All good, replace classes collection with sorted one and return
  classes_.swap(sortedClasses);
  return Status::Ok();
}

/// ClassNode
ClassNode::ClassNode(const std::string &className,
                     const std::string &parentClassName,
                     std::vector<AttributeNodePtr> attributes,
                     std::vector<MethodNodePtr> methods, const bool builtIn,
                     const uint32_t lloc, const uint32_t cloc)
    : ParentNode(lloc, cloc), builtIn_(builtIn), className_(className),
      parentClassName_(parentClassName), attributes_(std::move(attributes)),
      methods_(std::move(methods)) {}

ClassNodePtr ClassNode::MakeClassNode(
    const std::string &className, const std::string &parentClassName,
    std::vector<GenericAttributeNodePtr> genericAttributes, const bool builtIn,
    const uint32_t lloc, const uint32_t cloc) {
  /// Separate class methods from class attributes
  std::vector<AttributeNodePtr> attributes;
  std::vector<MethodNodePtr> methods;
  for (auto genericAttribute : genericAttributes) {
    if (std::dynamic_pointer_cast<AttributeNode>(genericAttribute)) {
      attributes.push_back(
          std::dynamic_pointer_cast<AttributeNode>(genericAttribute));
    } else {
      methods.push_back(
          std::dynamic_pointer_cast<MethodNode>(genericAttribute));
    }
  }

  /// Construct the class node
  return ClassNodePtr(new ClassNode(className, parentClassName,
                                    std::move(attributes), std::move(methods),
                                    builtIn, lloc, cloc));
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
                       std::vector<FormalNodePtr> arguments, ExprNodePtr body,
                       const uint32_t lloc, const uint32_t cloc)
    : ParentNode(lloc, cloc), id_(id), returnTypeName_(returnTypeName),
      arguments_(std::move(arguments)), body_(body) {}

MethodNodePtr MethodNode::MakeMethodNode(const std::string &id,
                                         const std::string &returnTypeName,
                                         std::vector<FormalNodePtr> arguments,
                                         ExprNodePtr body, const uint32_t lloc,
                                         const uint32_t cloc) {
  return MethodNodePtr(new MethodNode(id, returnTypeName, std::move(arguments),
                                      body, lloc, cloc));
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
