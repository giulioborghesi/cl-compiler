#include <cool/codegen/codegen_constant.h>
#include <cool/codegen/codegen_context.h>
#include <cool/codegen/codegen_helpers.h>
#include <cool/ir/expr.h>

namespace cool {

namespace {

Status GenerateStringLiteral(CodegenContext *context, std::string literal,
                             std::ostream *ios) {
  /// Nothing to do if string literal was already generated
  const size_t originalLength = literal.length();
  if (context->hasStringLabel(literal)) {
    return Status::Ok();
  }

  /// Add zero to string and pad string with zeros to word boundary
  literal.push_back('\0');
  while (literal.length() % WORD_SIZE) {
    literal.push_back('\0');
  }

  /// Emit string literal label
  const std::string label = context->generateStringLabel(literal);
  emit_object_label(label, ios);

  /// Emit class ID, object size, dispatch pointer and string data
  auto registry = context->classRegistry();
  const size_t typeID = registry->typeID("String");
  emit_word_data(typeID, ios);
  emit_word_data(literal.length() / 4 + 4, ios);
  emit_word_data("String_vtable", ios);
  emit_word_data(context->generateIntLabel(originalLength), ios);
  emit_ascii_data(literal, ios);
  return Status::Ok();
}

} // namespace

Status CodegenConstantPass::codegen(CodegenContext *context,
                                    LiteralExprNode<std::string> *node,
                                    std::ostream *ios) {
  return GenerateStringLiteral(context, node->value(), ios);
}

} // namespace cool