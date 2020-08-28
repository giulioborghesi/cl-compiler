#include <cool/codegen/codegen_constant.h>
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
std::vector<std::string> GLOBAL_LABELS = {
    "Main_protObj",        "Int_protObj",      "String_protObj", "_int_tag",
    "_bool_tag",           "_string_tag",      "bool_const0",    "bool_const1",
    "_MemMgr_INITIALIZER", "_MemMgr_COLLECTOR"};

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
                             std::string literal, std::ostream *ios) {
  /// Generate Int object for string length
  const size_t originalLength = literal.length();
  const std::string intLabel = context->generateIntLabel(originalLength);
  GenerateIntegerLiteral(context, intLabel, INT_TYPE, originalLength, ios);

  /// Add zero to string and pad string with zeros to word boundary
  literal.push_back('\0');
  while (literal.length() % WORD_SIZE) {
    literal.push_back('\0');
  }

  /// Emit string literal label
  emit_object_label(label, ios);

  /// Emit class ID, object size, dispatch pointer and string data
  auto registry = context->classRegistry();
  const size_t typeID = registry->typeID("String");
  emit_word_data(typeID, ios);
  emit_word_data(literal.length() / 4 + 4, ios);
  emit_word_data("String_dispTab", ios);
  emit_word_data(intLabel, ios);
  emit_ascii_data(literal, ios);
  return Status::Ok();
}

} // namespace

Status CodegenConstantPass::codegen(CodegenContext *context, ClassNode *node,
                                    std::ostream *ios) {
  const std::string label = node->className() + "_className";
  GenerateStringLiteral(context, label, node->className(), ios);
  return CodegenBasePass::codegen(context, node, ios);
}

Status CodegenConstantPass::codegen(CodegenContext *context,
                                    LiteralExprNode<int32_t> *node,
                                    std::ostream *ios) {
  const std::string label = context->generateIntLabel(node->value());
  return GenerateIntegerLiteral(context, label, INT_TYPE, node->value(), ios);
}

Status CodegenConstantPass::codegen(CodegenContext *context,
                                    LiteralExprNode<std::string> *node,
                                    std::ostream *ios) {
  const std::string label = context->generateStringLabel(node->value());
  return GenerateStringLiteral(context, label, node->value(), ios);
}

Status CodegenConstantPass::codegen(CodegenContext *context, ProgramNode *node,
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

  /// Emit class tags for Int, Bool and String types
  auto registry = context->classRegistry();
  std::vector<std::string> tags{"Int", "Bool", "String"};
  for (auto tag : tags) {
    const size_t classID = registry->typeID(tag);
    std::for_each(tag.begin(), tag.end(), [](char &c) { c = std::tolower(c); });
    emit_label("_" + tag + "_tag", ios);
    emit_word_data(classID, ios);
  }

  /// Generate prototype objects for Int, String and Bool objects
  GenerateIntegerLiteral(context, "Int_protObj", INT_TYPE, 0, ios);
  GenerateStringLiteral(context, "String_protObj", "", ios);
  GenerateIntegerLiteral(context, "bool_const0", BOOL_TYPE, 0, ios);
  GenerateIntegerLiteral(context, "bool_const1", BOOL_TYPE, 1, ios);
  return CodegenBasePass::codegen(context, node, ios);
}

} // namespace cool