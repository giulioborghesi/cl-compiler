#ifndef COOL_IR_CLASS_H
#define COOL_IR_CLASS_H

#include <cool/ir/node.h>
#include <cool/ir/visitable.h>

#include <memory>
#include <string>
#include <vector>

namespace cool {

/// Forward declarations
class ClassNode;

class AttributeNode {};
class MethodNode {};

/// Class for a node representing a COOL program
class ProgramNode {

public:
  ProgramNode() = delete;
  ~ProgramNode() = default;

  /// Factory method to create a class node
  ///
  /// classes vector of shared pointers to the nodes for the program classes
  /// \return a pointer to the new program node
  static ProgramNode *
  MakeProgramNode(std::vector<std::shared_ptr<ClassNode>> *classes);

  /// Get the nodes of the program classes
  ///
  /// \return a vector of shared pointers to the nodes for the program classes
  const std::vector<std::shared_ptr<ClassNode>> &classes() const;

private:
  ProgramNode(std::vector<std::shared_ptr<ClassNode>> *classes);

  const std::vector<std::shared_ptr<ClassNode>> classes_;
};

class ClassNode : public Visitable<Node, ClassNode> {

  using ParentNode = Visitable<Node, ClassNode>;

public:
  ClassNode() = delete;
  ~ClassNode() final override = default;

  /// Factory method to create a class node
  ///
  /// \param[in] className class name
  /// \param[in] parentClassName parent class name
  /// \param[in] attributes list of pointers to nodes of class attributes
  /// \param[in] methods list of pointers to nodes of class methods
  /// \param[in] lloc line location
  /// \param[in] cloc character location
  /// \return a pointer to the new class node
  static ClassNode *
  MakeClassNode(const std::string &className,
                const std::string &parentClassName,
                std::vector<std::shared_ptr<AttributeNode>> *attributes,
                std::vector<std::shared_ptr<MethodNode>> *methods,
                const uint32_t lloc, const uint32_t cloc);

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
  /// \return a vector of shared pointers to the nodes of the class attributes
  const std::vector<std::shared_ptr<AttributeNode>> &attributes() const {
    return attributes_;
  }

  /// Get the nodes of the class methods
  ///
  /// \return a vector of shared pointers to the nodes of the class methods
  const std::vector<std::shared_ptr<MethodNode>> &methods() const {
    return methods_;
  }

private:
  ClassNode(const std::string &className, const std::string &parentClassName,
            std::vector<std::shared_ptr<AttributeNode>> *attributes,
            std::vector<std::shared_ptr<MethodNode>> *methods,
            const uint32_t lloc, const uint32_t cloc);

  const std::string className_;
  const std::string parentClassName_;

  const std::vector<std::shared_ptr<AttributeNode>> attributes_;
  const std::vector<std::shared_ptr<MethodNode>> methods_;
};

} // namespace cool

#endif
