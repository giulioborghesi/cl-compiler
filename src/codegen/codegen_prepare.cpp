#include <cool/codegen/codegen_context.h>
#include <cool/codegen/codegen_helpers.h>
#include <cool/codegen/codegen_prepare.h>
#include <cool/ir/class.h>

#include <map>

namespace cool {

namespace {

/// \brief Generate the code for a table in the data section where the i-th
/// element points to the address of a String object for the name of Class with
/// class ID equal to i
///
/// \param[in] context Codegen context
/// \param[in] node program node
/// \param[out] ios output stream
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

/// \brief Generate the code for a table in the data section where the i-th
/// element is the class ID of the parent of the class with class ID equal to i
///
/// \param[in] context Codegen context
/// \param[in] node program node
/// \param[out] ios output stream
void GenerateClassHierarchyTable(CodegenContext *context, ProgramNode *node,
                                 std::ostream *ios) {
  /// Sort classes by ID
  auto registry = context->classRegistry();
  std::map<int32_t, int32_t> classToParentID;
  for (auto classNode : node->classes()) {
    const int32_t parentID =
        classNode->hasParentClass()
            ? registry->typeID(classNode->parentClassName())
            : -1;
    classToParentID[registry->typeID(classNode->className())] = parentID;
  }

  /// Generate class hierarchy table
  emit_label("class_parentTab", ios);
  for (auto it = classToParentID.begin(); it != classToParentID.end(); ++it) {
    emit_word_data(it->second, ios);
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

  /// Generate class hierarchy table
  GenerateClassHierarchyTable(context, node, ios);

  /// Generate class symbol tables
  for (auto classNode : node->classes()) {
    classNode->generateCode(context, this, ios);
  }
  return Status::Ok();
}

} // namespace cool
