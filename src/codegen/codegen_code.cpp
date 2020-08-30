#include <cool/codegen/codegen_code.h>
#include <cool/codegen/codegen_context.h>
#include <cool/codegen/codegen_helpers.h>
#include <cool/ir/class.h>
#include <cool/ir/expr.h>

namespace cool {

namespace {

/// Globally visible text labels
const std::vector<std::string> GLOBAL_LABELS = {"Main_init", "Main.main",
                                                "Int_init", "String_init"};

/// \brief Get attribute offset
///
/// \param[in] symbolTable symbol table
/// \param[in] attributeID attribute ID
/// \return the attribute offset in bytes
template <typename SymbolTableT>
int32_t GetAttributeOffset(SymbolTableT *symbolTable,
                           const std::string &attributeID) {
  auto position = symbolTable->get(attributeID).position;
  return WORD_SIZE * position + OBJECT_CONTENT_OFFSET;
}

/// \brief Store the new attribute value in the object and reset the accumulator
/// register to self.
///
/// The new attribute value is expected to be stored in register $a0.
///
/// \param[in] offset attribute offset in bytes
/// \param[out] ios output stream
void StoreAttributeAndSetAccumulatorToSelf(const int32_t offset,
                                           std::ostream *ios) {
  emit_lw_instruction("$t0", "$fp", 0, ios);
  emit_sw_instruction("$a0", "$t0", offset, ios);
  emit_move_instruction("$a0", "$t0", ios);
}

} // namespace

Status CodegenObjectsInitPass::codegen(CodegenContext *context,
                                       AttributeNode *node, std::ostream *ios) {
  /// If attribute has an initialization expression, use it
  if (node->initExpr()) {
    auto symbolTable = context->symbolTable();
    const int32_t offset = GetAttributeOffset(symbolTable, node->id());
    node->initExpr()->generateCode(context, this, ios);
    StoreAttributeAndSetAccumulatorToSelf(offset, ios);
  }
  return Status::Ok();
}

Status CodegenObjectsInitPass::codegen(CodegenContext *context, ClassNode *node,
                                       std::ostream *ios) {
  /// Set current class name in context and fetch symbol table
  context->resetStackPosition();
  context->setCurrentClassName(node->className());
  auto symbolTable = context->symbolTable();

  /// Generate init label. Nothing to do for built-in classes
  emit_label(node->className() + "_init", ios);
  if (node->builtIn() && node->className() != "String") {
    emit_jump_and_link_instruction("$ra", ios);
    return Status::Ok();
  }

  /// Load attributes in symbol table
  for (auto attributeNode : node->attributes()) {
    const size_t attributePosition = symbolTable->count();
    IdentifierCodegenInfo attributeInfo(true, attributePosition);
    symbolTable->addElement(attributeNode->id(), attributeInfo);
  }

  /// Push stack frame
  PushStackFrame(context, ios);

  /// Initialize parent class if needed
  if (node->hasParentClass()) {
    const std::string label = node->parentClassName() + "_init";
    emit_jump_and_link_instruction(label, ios);
  }

  /// Initialize attributes to default values
  for (auto attribute : node->attributes()) {
    const std::string &type = attribute->typeName();
    if (type == "Int" || type == "String" || type == "Bool") {
      const int32_t offset = GetAttributeOffset(symbolTable, attribute->id());
      emit_lw_instruction("$a0", "$a0", offset, ios);
      emit_jump_and_link_instruction("Object.copy", ios);
      StoreAttributeAndSetAccumulatorToSelf(offset, ios);
    }
  }

  /// Initialize attributes
  for (auto attribute : node->attributes()) {
    attribute->generateCode(context, this, ios);
  }

  /// Restore calling stack frame and return control to caller
  PopStackFrame(context, ios);
  emit_jump_register_instruction("$ra", ios);

  /// Generate code for remaining methods
  for (auto methodNode : node->methods()) {
    methodNode->generateCode(context, this, ios);
  }
  return Status::Ok();
}

Status CodegenObjectsInitPass::codegen(CodegenContext *context,
                                       ProgramNode *node, std::ostream *ios) {
  /// Emit heap start
  emit_label("heap_start", ios);
  emit_word_data(0, ios);

  /// Emit text directive
  emit_directive(".text", ios);

  /// Emit global declarations for text labels
  for (const auto &label : GLOBAL_LABELS) {
    emit_global_declaration(label, ios);
  }

  /// Traverse each class
  return CodegenBasePass::codegen(context, node, ios);
}

} // namespace cool
