#include <cool/codegen/codegen_context.h>
#include <cool/codegen/codegen_data.h>
#include <cool/codegen/codegen_helpers.h>
#include <cool/ir/class.h>

#include <map>

namespace cool {

Status CodegenDataPass::codegen(CodegenContext *context, ClassNode *node,
                                std::ostream *ios) {
  /// Set current class name
  context->setCurrentClassName(node->className());

  /// Create prototype object
  emit_label(node->className() + "_protObj", ios);

  /// Write class tag
  auto registry = context->classRegistry();
  const size_t classID = registry->typeID(node->className());
  emit_word_data(classID, ios);

  /// Write object size
  auto symbolTable = context->symbolTable();
  const size_t nAttributes = symbolTable->count();
  emit_word_data(nAttributes + 3, ios);

  /// Write dispatch pointer
  emit_word_data(node->className() + "_vtable", ios);

  /// Initialize all attributes to zero
  for (size_t i = 0; i < nAttributes; ++i) {
    emit_word_data(0, ios);
  }

  return Status::Ok();
}

Status CodegenDataPass::codegen(CodegenContext *context, ProgramNode *node,
                                std::ostream *ios) {
  emit_directive(".data", ios);

  /// Generate inheritance tree
  std::map<size_t, int32_t> classTree;
  auto registry = context->classRegistry();
  for (auto classNode : node->classes()) {
    const size_t classID = registry->typeID(classNode->className());
    if (classNode->hasParentClass()) {
      classTree[classID] = registry->typeID(classNode->parentClassName());
    } else {
      classTree[classID] = -1;
    }
  }

  /// Write inheritance tree
  emit_label("Classes_ancestors", ios);
  for (auto it = classTree.begin(); it != classTree.end(); ++it) {
    emit_word_data(it->second, ios);
  }

  /// Generate objects prototypes and vtables
  for (auto classNode : node->classes()) {
    classNode->generateCode(context, this, ios);
  }

  return Status::Ok();
}

} // namespace cool
