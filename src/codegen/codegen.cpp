#include <cool/codegen/codegen.h>
#include <cool/codegen/codegen_helpers.h>
#include <cool/core/context.h>
#include <cool/ir/expr.h>

namespace cool {

Status CodegenPass::codegen(Context *context, BlockExprNode *node) {
  for (auto expr : node->exprs()) {
    expr->generateCode(context, this);
  }
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

} // namespace cool
