#ifndef COOL_IR_CLASS_H
#define COOL_IR_CLASS_H

#include <cool/core/status.h>
#include <cool/ir/common.h>
#include <cool/ir/fwd.h>
#include <cool/ir/node.h>
#include <cool/ir/visitable.h>

#include <memory>
#include <string>
#include <vector>

namespace cool {

/// Class for a node representing a COOL program
class ProgramNode : public Visitable<Node, ProgramNode> {

  using ParentNode = Visitable<Node, ProgramNode>;

public:
  ProgramNode() = delete;
  ~ProgramNode() = default;

  /// Factory method to create a class node
  ///
  /// classes vector of shared pointers to the nodes for the program classes
  /// \return a shared pointer to the new program node
  static ProgramNodePtr MakeProgramNode(std::vector<ClassNodePtr> classes);

  /// Get the nodes of the program classes
  ///
  /// \return a vector of shared pointers to the nodes for the program classes
  const std::vector<ClassNodePtr> &classes() const { return classes_; }

  /// Sort the program classes in topological order
  ///
  /// \return Status::Ok() if the classes tree has no cycle, an error otherwise
  Status sortClasses();

private:
  ProgramNode(std::vector<ClassNodePtr> classes);

  std::vector<ClassNodePtr> classes_;
};

/// Class for a node representing a COOL class
class ClassNode : public Visitable<Node, ClassNode> {

  using ParentNode = Visitable<Node, ClassNode>;

public:
  ClassNode() = delete;
  ~ClassNode() final override = default;

  /// Factory method to create a class node
  ///
  /// \param[in] className class name
  /// \param[in] parentClassName parent class name
  /// \param[in] genericAttributes list of shared pointers to attribute nodes
  /// \param[in] builtIn built-in class parameter
  /// \param[in] lloc line location
  /// \param[in] cloc character
  /// location \return a shared pointer to the new class node
  static ClassNodePtr
  MakeClassNode(const std::string &className,
                const std::string &parentClassName,
                std::vector<GenericAttributeNodePtr> genericAttributes,
                const bool builtIn, const uint32_t lloc, const uint32_t cloc);

  /// Query whether the class is a built-in class or not
  ///
  /// \return True if the class is a built-in class, false otherwise
  bool builtIn() const { return builtIn_; }

  /// Get the class name
  ///
  /// \return the class name
  const std::string &className() const { return className_; }

  /// Get the parent class name
  ///
  /// \return the parent class name
  const std::string &parentClassName() const { return parentClassName_; };

  /// Check whether the class inherits from a parent class
  ///
  /// \return true if the class inherits from a parent class, false otherwise
  bool hasParentClass() const { return parentClassName_.length() > 0; }

  /// Get the nodes of the class attributes
  ///
  /// \return a vector of shared pointers to the class attributes nodes
  const std::vector<AttributeNodePtr> &attributes() const {
    return attributes_;
  }

  /// Get the nodes of the class methods
  ///
  /// \return a vector of shared pointers to the class method nodes
  const std::vector<MethodNodePtr> &methods() const { return methods_; }

private:
  ClassNode(const std::string &className, const std::string &parentClassName,
            std::vector<AttributeNodePtr> attributes,
            std::vector<MethodNodePtr> methods, const bool builtIn,
            const uint32_t lloc, const uint32_t cloc);

  const bool builtIn_;

  const std::string className_;
  const std::string parentClassName_;

  const std::vector<AttributeNodePtr> attributes_;
  const std::vector<MethodNodePtr> methods_;
};

/// Class for a node representing a generic attribute in a COOL class
class GenericAttributeNode : public Node {

public:
  GenericAttributeNode() = delete;
  GenericAttributeNode(const uint32_t lloc, const uint32_t cloc)
      : Node(lloc, cloc) {}
  ~GenericAttributeNode() override = default;
};

/// Class for a node representing an attribute in a COOL class
class AttributeNode : public Visitable<GenericAttributeNode, AttributeNode> {

  using ParentNode = Visitable<GenericAttributeNode, AttributeNode>;

public:
  AttributeNode() = delete;
  ~AttributeNode() final override = default;

  static AttributeNodePtr MakeAttributeNode(const std::string &id,
                                            const std::string &typeName,
                                            ExprNodePtr initExpr,
                                            const uint32_t lloc,
                                            const uint32_t cloc);

  const std::string &id() const { return id_; }

  ExprNodePtr initExpr() const { return initExpr_; }

  const std::string &typeName() const { return typeName_; }

private:
  AttributeNode(const std::string &id, const std::string &typeName,
                ExprNodePtr initExpr, const uint32_t lloc, const uint32_t cloc);

  const std::string id_;
  const std::string typeName_;
  const ExprNodePtr initExpr_;
};

class MethodNode : public Visitable<GenericAttributeNode, MethodNode> {

  using ParentNode = Visitable<GenericAttributeNode, MethodNode>;

public:
  MethodNode() = delete;
  ~MethodNode() final override = default;

  static MethodNodePtr MakeMethodNode(const std::string &id,
                                      const std::string &returnTypeName,
                                      std::vector<FormalNodePtr> arguments,
                                      const uint32_t lloc, const uint32_t cloc);

  const std::vector<FormalNodePtr> &arguments() const { return arguments_; }

  const std::string &id() const { return id_; }

  const std::string &returnTypeName() const { return returnTypeName_; }

private:
  MethodNode(const std::string &id, const std::string &returnTypeName,
             std::vector<FormalNodePtr> arguments, const uint32_t lloc,
             const uint32_t cloc);

  const std::string id_;
  const std::string returnTypeName_;
  const std::vector<FormalNodePtr> arguments_;
};

class FormalNode : public Visitable<Node, FormalNode> {

  using ParentNode = Visitable<Node, FormalNode>;

public:
  FormalNode() = delete;
  ~FormalNode() final override = default;

  static FormalNodePtr MakeFormalNode(const std::string &id,
                                      const std::string &typeName,
                                      const uint32_t lloc, const uint32_t cloc);

  const std::string &id() const { return id_; }

  const std::string &typeName() const { return typeName_; }

private:
  FormalNode(const std::string &id, const std::string &typeName,
             const uint32_t lloc, const uint32_t cloc);

  const std::string id_;
  const std::string typeName_;
};

} // namespace cool

#endif
