#include <cool/codegen/codegen_context.h>
#include <cool/codegen/codegen_helpers.h>
#include <cool/codegen/codegen_prepare.h>
#include <cool/ir/class.h>

#include <map>

namespace cool {

namespace {

void GenerateClassNameTable(CodegenContext *context, ProgramNode *node,
                            std::ostream *ios) {
  /// Sort classes by ID
  auto registry = context->classRegistry();
  std::map<int32_t, std::string> idToName;
  for (auto classNode : node->classes()) {
    idToName[registry->typeID(classNode->className())] = classNode->className();
  }

  /// Generate class name table
  emit_label("class_nameTab", ios);
  for (auto it = idToName.begin(); it != idToName.end(); ++it) {
    const std::string label = it->second + "_className";
    emit_word_data(label, ios);
  }
}

} // namespace

Status CodegenPreparePass::codegen(CodegenContext *context, ClassNode *node,
                                   std::ostream *ios) {
  /// Initialize symbol table and method table
  context->setCurrentClassName(node->className());
  context->initializeTables();

  /// Compute the position of each method in the dispatch table
  auto methodTable = context->methodTable();
  for (auto methodNode : node->methods()) {
    size_t methodPosition = 0;
    if (!methodTable->findKey(methodNode->id())) {
      methodPosition = methodTable->count();
    } else {
      methodPosition = methodTable->get(methodNode->id()).position;
    }
    MethodCodegenInfo methodInfo(node->className(), methodPosition);
    methodTable->addElement(methodNode->id(), methodInfo);
  }

  /// Assemble the dispatch table
  std::map<size_t, std::string> methods;
  for (auto it = methodTable->begin(); it != methodTable->end(); ++it) {
    const std::string label = it->second.className + "." + it->first;
    methods[it->second.position] = label;
  }

  /// Generate code for the dispatch table
  emit_label(node->className() + "_dispTab", ios);
  for (auto it = methods.begin(); it != methods.end(); ++it) {
    emit_word_data(it->second, ios);
  }

  return Status::Ok();
}

Status CodegenPreparePass::codegen(CodegenContext *context, ProgramNode *node,
                                   std::ostream *ios) {
  /// Generate class names table
  GenerateClassNameTable(context, node, ios);

  /// Generate class symbol tables
  for (auto classNode : node->classes()) {
    classNode->generateCode(context, this, ios);
  }
  return Status::Ok();
}

} // namespace cool
