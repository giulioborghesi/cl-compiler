#include <cool/codegen/codegen_context.h>
#include <cool/codegen/codegen_helpers.h>
#include <cool/codegen/codegen_prepare.h>
#include <cool/ir/class.h>

namespace cool {

Status CodegenPreparePass::codegen(CodegenContext *context, ClassNode *node,
                                   std::ostream *ios) {
  /// Initialize symbol table and method table
  context->setCurrentClassName(node->className());
  context->initializeTables();

  /// Initialize the location of each method
  auto methodTable = context->methodTable();
  for (auto methodNode : node->methods()) {
    if (!methodTable->findKeyInTable(methodNode->id())) {
      const size_t methodPosition = methodTable->count();
      MethodCodegenInfo methodInfo(methodPosition);
      methodTable->addElement(methodNode->id(), methodInfo);
    }
  }

  return Status::Ok();
}

Status CodegenPreparePass::codegen(CodegenContext *context, ProgramNode *node,
                                   std::ostream *ios) {
  for (auto classNode : node->classes()) {
    classNode->generateCode(context, this, ios);
  }
  return Status::Ok();
}

} // namespace cool
