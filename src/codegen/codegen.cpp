#include <cool/codegen/codegen.h>
#include <cool/codegen/codegen_helpers.h>
#include <cool/core/context.h>
#include <cool/ir/expr.h>

namespace cool {

namespace {

/// Helper function that stores in register $a0 the address where the pointer to
/// the object identified by an identifier is stored
///
/// \param[in] context Codegen context
/// \param[in] id identifier name
/// \param[out] ios output stream
void FetchObjectPointerLocationFromID(Context *context, const std::string &id,
                                      std::iostream *ios) {
  /*  if (symbolInfo.isAttribute) {
      emit_lw_instruction("$t1", "$fp", 0, nullptr);
      emit_la_instruction("$a0", "$t1", symbolInfo.offset, nullptr);
    } else {
      emit_la_instruction("$a0", "$fp", symbolInfo.offset, nullptr);
    }
  */

  /// TODO: implement functionality - look at skeleton above
}

/// Helper function that returns a pointer to a default object for the specified
/// class in $a0. For classes other than built-in classes, $a0 will contain a
/// void pointer
///
/// \param[in] context Codegen context
/// \param[in] typeName class of default object
/// \param[out] ios output stream
void GenerateCodeForDefaultObject(Context *context, const std::string &typeName,
                                  std::iostream *ios) {
  /// TODO: implement functionality
}

} // namespace

Status CodegenPass::codegen(Context *context, BlockExprNode *node) {
  for (auto expr : node->exprs()) {
    expr->generateCode(context, this);
  }
  return Status::Ok();
}

Status CodegenPass::codegen(Context *context, AssignmentExprNode *node) {
  /// Evaluate right hand side expression
  node->rhsExpr()->generateCode(context, this);
  push_accumulator_to_stack(nullptr);

  /// Fetch location of object pointer given variable ID
  FetchObjectPointerLocationFromID(context, node->id(), nullptr);

  /// Update object pointed by variable
  emit_lw_instruction("$t1", "$sp", WORD_SIZE, nullptr);
  emit_sw_instruction("$t1", "$a0", 0, nullptr);

  /// Pop stack and return
  emit_addiu_instruction("$sp", "$sp", WORD_SIZE, nullptr);
  return Status::Ok();
}

Status CodegenPass::codegen(Context *context,
                            BinaryExprNode<ArithmeticOpID> *node) {
  /// Evaluate left and right hand side expressions
  node->lhsExpr()->generateCode(context, this);
  push_accumulator_to_stack(nullptr);
  node->rhsExpr()->generateCode(context, this);
  push_accumulator_to_stack(nullptr);

  /// Store lhs value on stack
  emit_lw_instruction("$t0", "$sp", 2 * WORD_SIZE, nullptr);
  emit_lw_instruction("$a0", "$t0", OBJECT_CONTENT_OFFSET, nullptr);
  push_accumulator_to_stack(nullptr);

  /// Store rhs value in accumulator
  emit_lw_instruction("$t0", "$sp", 2 * WORD_SIZE, nullptr);
  emit_lw_instruction("$a0", "$t0", OBJECT_CONTENT_OFFSET, nullptr);

  /// Sum values and store result on stack
  emit_lw_instruction("$t0", "$sp", WORD_SIZE, nullptr);
  emit_three_registers_instruction("add", "$a0", "$t0", "$a0", nullptr);
  push_accumulator_to_stack(nullptr);

  /// Allocate new integer object and store pointer in $a0

  /// Save computed value in new integer object
  emit_lw_instruction("$t0", "$sp", WORD_SIZE, nullptr);
  emit_sw_instruction("$t0", "$a0", OBJECT_CONTENT_OFFSET, nullptr);

  /// Pop stack and return
  emit_addiu_instruction("$sp", "$sp", 4 * WORD_SIZE, nullptr);
  return Status::Ok();
}

Status CodegenPass::codegen(Context *context, IfExprNode *node) {
  /// Generate labels
  const std::string trueLabel;
  const std::string endLabel;

  /// Emit code for if expression
  node->ifExpr()->generateCode(context, this);

  /// Load boolean value. Branch if true
  emit_lw_instruction("$t0", "$a0", OBJECT_CONTENT_OFFSET, nullptr);
  emit_bgtz_instruction("$t0", trueLabel, nullptr);

  /// Emit code for then expression
  node->thenExpr()->generateCode(context, this);
  emit_jump_instruction(endLabel, nullptr);

  /// Emit label for true branch
  emit_label(trueLabel, nullptr);

  /// Emit code for else expression
  node->elseExpr()->generateCode(context, this);

  /// Emit label for end of if construct and return
  emit_label(endLabel, nullptr);
  return Status::Ok();
}

Status CodegenPass::codegen(Context *context, LetBindingNode *node) {
  /// Generate code for right hand side expression
  if (node->hasExpr()) {
    node->expr()->generateCode(context, this);
  } else {
    GenerateCodeForDefaultObject(context, node->typeName(), nullptr);
  }

  /// Store pointer to object in stack
  push_accumulator_to_stack(nullptr);

  /// TODO: update environment with location of new variable
  return Status::Ok();
}

Status CodegenPass::codegen(Context *context, LetExprNode *node) {

  auto unwindStackAndEnvironment = [&](const uint32_t nCount) {
    emit_addiu_instruction("$sp", "$sp", nCount * WORD_SIZE, nullptr);
    for (uint32_t iCount = 0; iCount < nCount; ++iCount) {
      /// TODO: implement logic to unwind nested environments
    }
  };

  /// Generate code for let bindings
  for (auto binding : node->bindings()) {
    binding->generateCode(context, this);
  }

  /// Generate code for main let expression
  node->expr()->generateCode(context, this);

  /// Unwind stack and environment
  unwindStackAndEnvironment(node->bindings().size());
  return Status::Ok();
}

Status CodegenPass::codegen(Context *context, WhileExprNode *node) {
  /// Generate labels
  const std::string loopStartLabel;
  const std::string loopEndLabel;

  /// Emit label for start of loop
  emit_label(loopStartLabel, nullptr);

  /// Evaluate loop condition and branch if needed
  node->loopCond()->generateCode(context, this);
  emit_lw_instruction("$t0", "$a0", OBJECT_CONTENT_OFFSET, nullptr);
  emit_blez_instruction("$t0", loopEndLabel, nullptr);

  /// Generate code for loop body and jump to start of loop
  node->loopBody()->generateCode(context, this);
  emit_jump_instruction(loopStartLabel, nullptr);

  /// Emit label for end of loop construct
  emit_label(loopEndLabel, nullptr);

  /// Void the return value and return
  emit_move_instruction("$a0", "$zero", nullptr);
  return Status::Ok();
}

} // namespace cool
