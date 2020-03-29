#include <cool/analysis/pass.h>
#include <cool/ir/class.h>

namespace cool {

ProgramNode::ProgramNode(std::vector<std::shared_ptr<ClassNode>> *classes)
    : classes_(std::move(*classes)) {}

ProgramNode *
ProgramNode::MakeProgramNode(std::vector<std::shared_ptr<ClassNode>> *classes) {
  return new ProgramNode(classes);
}

ClassNode::ClassNode(const std::string &className,
                     const std::string &parentClassName,
                     std::vector<std::shared_ptr<AttributeNode>> *attributes,
                     std::vector<std::shared_ptr<MethodNode>> *methods,
                     const uint32_t lloc, const uint32_t cloc)
    : Node(lloc, cloc), className_(className),
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

} // namespace cool
