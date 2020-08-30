#include <cool/codegen/codegen_constants.h>
#include <cool/codegen/codegen_context.h>
#include <cool/codegen/codegen_helpers.h>
#include <cool/ir/class.h>
#include <cool/ir/expr.h>

#include <vector>

namespace cool {

namespace {

/// Integer types supported in Cool
const std::string BOOL_TYPE = "Bool";
const std::string INT_TYPE = "Int";

/// Globally visible data labels
const std::vector<std::string> GLOBAL_LABELS = {
    "Main_protObj",        "Int_protObj",       "String_protObj", "_int_tag",
    "_bool_tag",           "_string_tag",       "Bool_const0",    "Bool_const1",
    "_MemMgr_INITIALIZER", "_MemMgr_COLLECTOR", "_MemMgr_TEST",   "heap_start"};

Status GenerateBuiltInPrototype(CodegenContext *context,
                                const std::string &type, std::ostream *ios) {
  /// Emit literal label
  emit_object_label(type + "_protObj", ios);

  /// Emit class ID, object size and dispatch pointer
  auto registry = context->classRegistry();
  const size_t typeID = registry->typeID(type);
  emit_word_data(typeID, ios);
  emit_word_data(3, ios);
  emit_word_data(type + "_dispTab", ios);
  return Status::Ok();
}

/// \brief Helper function to generate an object for an integer literal.
/// Supported integer literals in Cool are Bool and Int
///
/// \param[in] context Codegen context
/// \param[in] label object label
/// \param[in] intType integer type (Int or Bool)
/// \param[in] literal literal value
/// \param[out] ios output stream
/// \return Status::Ok()
Status GenerateIntegerLiteral(CodegenContext *context, const std::string &label,
                              const std::string &intType, const int32_t literal,
                              std::ostream *ios) {
  /// Emit literal label
  emit_object_label(label, ios);

  /// Emit class ID, object size, dispatch pointer and data
  auto registry = context->classRegistry();
  const size_t typeID = registry->typeID(intType);
  emit_word_data(typeID, ios);
  emit_word_data(4, ios);
  emit_word_data(intType + "_dispTab", ios);
  emit_word_data(literal, ios);
  return Status::Ok();
}

/// \brief Helper function to generate an object for a String literal
///
/// \param[in] context Codegen context
/// \param[in] label object label
/// \param[in] literal string literal
/// \param[out] ios output stream
/// \return Status::Ok()
Status GenerateStringLiteral(CodegenContext *context, const std::string &label,
                             const std::string &literal, std::ostream *ios) {
  /// Generate Int object for string length
  const size_t originalLength = literal.length();
  if (!context->hasIntLabel(originalLength)) {
    const std::string intLabel = context->generateIntLabel(originalLength);
    GenerateIntegerLiteral(context, intLabel, INT_TYPE, originalLength, ios);
  }
  const std::string intLabel = context->generateIntLabel(originalLength);

  /// Emit string literal label
  emit_object_label(label, ios);

  /// Emit class ID, object size, dispatch pointer and string data
  auto registry = context->classRegistry();
  const size_t typeID = registry->typeID("String");
  emit_word_data(typeID, ios);
  emit_word_data(5 + literal.length() / 4, ios);
  emit_word_data("String_dispTab", ios);
  emit_word_data(intLabel, ios);
  emit_ascii_data(literal, ios);
  emit_byte_data(0, ios);
  emit_align_data(2, ios);
  return Status::Ok();
}

} // namespace

Status CodegenConstantsPass::codegen(CodegenContext *context, ClassNode *node,
                                     std::ostream *ios) {
  const std::string label = node->className() + "_className";
  GenerateStringLiteral(context, label, node->className(), ios);
  return CodegenBasePass::codegen(context, node, ios);
}

Status CodegenConstantsPass::codegen(CodegenContext *context,
                                     LiteralExprNode<int32_t> *node,
                                     std::ostream *ios) {
  if (context->hasIntLabel(node->value())) {
    return Status::Ok();
  }
  const std::string label = context->generateIntLabel(node->value());
  return GenerateIntegerLiteral(context, label, INT_TYPE, node->value(), ios);
}

Status CodegenConstantsPass::codegen(CodegenContext *context,
                                     LiteralExprNode<std::string> *node,
                                     std::ostream *ios) {
  if (context->hasStringLabel(node->value())) {
    return Status::Ok();
  }
  const std::string label = context->generateStringLabel(node->value());
  return GenerateStringLiteral(context, label, node->value(), ios);
}

Status CodegenConstantsPass::codegen(CodegenContext *context, ProgramNode *node,
                                     std::ostream *ios) {
  /// Emit data directive
  emit_directive(".data", ios);

  /// Emit global declarations for data labels
  for (const auto &label : GLOBAL_LABELS) {
    emit_global_declaration(label, ios);
  }

  /// Emit GC initializer settings
  emit_label("_MemMgr_INITIALIZER", ios);
  emit_word_data("_NoGC_Init", ios);

  /// Emit GC collector settings
  emit_label("_MemMgr_COLLECTOR", ios);
  emit_word_data("_NoGC_Collect", ios);

  /// Emit memory manager settings
  emit_label("_MemMgr_TEST", ios);
  emit_word_data(0, ios);

  /// Emit class tags for Int, Bool and String types
  auto registry = context->classRegistry();
  std::vector<std::string> tags{"Int", "Bool", "String"};
  for (auto tag : tags) {
    const size_t classID = registry->typeID(tag);
    std::for_each(tag.begin(), tag.end(), [](char &c) { c = std::tolower(c); });
    emit_label("_" + tag + "_tag", ios);
    emit_word_data(classID, ios);
  }

  /// Generate prototype objects for Object and IO
  GenerateBuiltInPrototype(context, "Object", ios);
  GenerateBuiltInPrototype(context, "IO", ios);

  /// Generate prototype objects for Int, String and Bool objects
  GenerateIntegerLiteral(context, "Int_protObj", INT_TYPE, 0, ios);
  GenerateStringLiteral(context, "String_protObj", "", ios);
  GenerateIntegerLiteral(context, "Bool_protObj", BOOL_TYPE, 0, ios);
  GenerateIntegerLiteral(context, "Bool_const0", BOOL_TYPE, 0, ios);
  GenerateIntegerLiteral(context, "Bool_const1", BOOL_TYPE, 1, ios);
  return CodegenBasePass::codegen(context, node, ios);
}

} // namespace cool