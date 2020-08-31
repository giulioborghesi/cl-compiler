#include <cool/codegen/codegen_context.h>
#include <cool/codegen/codegen_helpers.h>
#include <cool/codegen/codegen_tables.h>
#include <cool/ir/class.h>

#include <map>

namespace cool {

namespace {

/// Mapping from type name to label for default value
const std::unordered_map<std::string, std::string> TYPE_TO_DEFAULT_VALUE{
    {"String", "String_protObj"}, {"Int", "Int_protObj"}, {"Bool", BOOL_FALSE}};

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
  emit_label(CLASS_NAME_TABLE, ios);
  for (auto it = idToName.begin(); it != idToName.end(); ++it) {
    const std::string label = it->second + "_className";
    emit_word_data(label, ios);
  }
}

/// \brief Generate the code for a table in the data section where the i-th
/// element points to the address of the i-th class dispatch table
///
/// \param[in] context Codegen context
/// \param[in] node program node
/// \param[out] ios output stream
void GenerateClassDispatchTableIndexTable(CodegenContext *context,
                                          ProgramNode *node,
                                          std::ostream *ios) {
  /// Sort classes by ID
  auto registry = context->classRegistry();
  std::map<int32_t, std::string> idToName;
  for (auto classNode : node->classes()) {
    idToName[registry->typeID(classNode->className())] = classNode->className();
  }

  /// Generate class dispatch table index table
  emit_label(DISPATCH_TABLE_INDEX_TABLE, ios);
  for (auto it = idToName.begin(); it != idToName.end(); ++it) {
    const std::string label = it->second + "_dispTab";
    emit_word_data(label, ios);
  }
}

/// \brief Generate the default value for a class attribute
///
/// \param[in] node attribute node
/// \param[out] ios output stream
void GenerateDefaultAttributeValue(AttributeNode *node, std::ostream *ios) {
  if (TYPE_TO_DEFAULT_VALUE.count(node->typeName())) {
    emit_word_data(TYPE_TO_DEFAULT_VALUE.find(node->typeName())->second, ios);
  } else {
    emit_word_data(0, ios);
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
  emit_label(CLASS_PARENT_TABLE, ios);
  for (auto it = classToParentID.begin(); it != classToParentID.end(); ++it) {
    emit_word_data(it->second, ios);
  }
}

} // namespace

Status CodegenTablesPass::codegen(CodegenContext *context, ClassNode *node,
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

  /// Nothing to do for built-in classes
  if (node->builtIn()) {
    return Status::Ok();
  }

  /// Initialize ancestors vector
  auto currentNode = node;
  std::vector<ClassNode *> nodes{currentNode};

  /// Fetch ancestors
  size_t nAttributes = currentNode->attributes().size();
  auto registry = context->classRegistry();
  while (currentNode->hasParentClass()) {
    currentNode = registry->classNode(currentNode->parentClassName()).get();
    nAttributes += currentNode->attributes().size();
    nodes.push_back(currentNode);
  }
  std::reverse(nodes.begin(), nodes.end());

  /// Generate code for prototype object
  emit_object_label(node->className() + "_protObj", ios);
  emit_word_data(registry->typeID(node->className()), ios);
  emit_word_data(nAttributes + 3, ios);
  emit_word_data(node->className() + "_dispTab", ios);
  for (auto node : nodes) {
    for (auto attributeNode : node->attributes()) {
      GenerateDefaultAttributeValue(attributeNode.get(), ios);
    }
  }
  return Status::Ok();
}

Status CodegenTablesPass::codegen(CodegenContext *context, ProgramNode *node,
                                  std::ostream *ios) {
  /// Generate class names table
  GenerateClassNameTable(context, node, ios);

  /// Generate class hierarchy table
  GenerateClassHierarchyTable(context, node, ios);

  /// Generate class dispatch table index table
  GenerateClassDispatchTableIndexTable(context, node, ios);

  /// Generate class symbol tables and prototype objects
  for (auto classNode : node->classes()) {
    classNode->generateCode(context, this, ios);
  }
  return Status::Ok();
}

} // namespace cool
