#include <cool/codegen/codegen.h>
#include <cool/codegen/codegen_helpers.h>
#include <cool/core/context.h>
#include <cool/ir/expr.h>

namespace cool {

namespace {

/// Forward declaration
void CreateObjectFromProto(Context *context, const std::string &protoLabel,
                           const std::string &initLabel, std::iostream *ios);

/// Helper function to create an integer object storing a given int value
///
/// \param[in] context Codegen context
/// \param[in] value value of Int object
/// \param[in] ios output stream
void CreateIntObject(Context *context, const int32_t value,
                     std::iostream *ios) {
  CreateObjectFromProto(context, "Int_protObj", "Int_init", ios);
  emit_li_instruction("$t0", value, ios);

  /// Update Int value and return
  emit_sw_instruction("$t0", "$a0", OBJECT_CONTENT_OFFSET, ios);
}

/// Helper function to create a copy of an object given the labels for its
/// prototype and its init function. The pointer to the newly created object is
/// stored in register $a0
///
/// \param[in] context Codegen context
/// \param[in] protoLabel prototype object label
/// \param[in] initLabel object init label
/// \param[out] ios output stream
void CreateObjectFromProto(Context *context, const std::string &protoLabel,
                           const std::string &initLabel, std::iostream *ios) {
  /// Load address of prototype object into $a0
  emit_la_instruction("$a0", protoLabel, ios);

  /// Create a copy of the prototype object and store it on the stack
  emit_jump_and_link_instruction("Object.copy", ios);
  push_accumulator_to_stack(ios);

  /// Initialize object
  emit_jump_and_link_instruction(initLabel, ios);

  /// Store address of new object in accumulator register and restore stack
  emit_lw_instruction("$a0", "$sp", WORD_SIZE, ios);
  emit_addiu_instruction("$sp", "$sp", WORD_SIZE, ios);
}

/// Helper function to create a copy of an object given its unique identifyer.
/// The pointer to the newly created object is stored in register $a0
///
/// \param[in] context Codegen context
/// \param[in] classID class unique ID
/// \param[out] ios output stream
void CreateObjectFromTypeID(Context *context, const IdentifierType classID,
                            std::iostream *ios) {
  /// Compute offsets in prototype and init tables
  // emit_li_instruction("$t0", (int32_t)classID, ios);
  // emit_sll_instruction("$t0", "$t0", 3, ios);

  /// Load address of prototype object into $a0
  emit_la_instruction("$t1", "Proto_table", ios);
  emit_three_registers_instruction("addu", "$t1", "$t0", "$t1", ios);
  emit_lw_instruction("$a0", "$t1", 0, ios);

  /// Create a copy of the prototype object and store it on the stack
  emit_jump_and_link_instruction("Object.copy", ios);
  push_accumulator_to_stack(ios);

  /// Load address of init function into $t1 and initialize object
  emit_la_instruction("$t1", "Init_table", ios);
  emit_three_registers_instruction("addu", "$t1", "$t0", "$t1", ios);
  emit_jump_and_link_instruction(
      "$t1", ios); // TODO: use jalr instead of jal - refactor this code

  /// Store address of new object in accumulator register and restore stack
  emit_lw_instruction("$a0", "$sp", WORD_SIZE, ios);
  emit_addiu_instruction("$sp", "$sp", WORD_SIZE, ios);
}

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

std::string GetMnemonicFromOpType(const ArithmeticOpID opID) {
  switch (opID) {
  case ArithmeticOpID::Plus:
    return "add";
  case ArithmeticOpID::Minus:
    return "sub";
  case ArithmeticOpID::Mult:
    return "mul";
  case ArithmeticOpID::Div:
    return "div";
  }
  return "";
}

std::string GetMnemonicFromOpType(const ComparisonOpID opID) {
  return opID == ComparisonOpID::LessThan ? "blt" : "ble";
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

/// Helper function that stores the case jump address in register $a0
void SelectCaseStatement(Context *context, CaseExprNode *node) {
  /// Load object address into $t0
  emit_lw_instruction("$t0", "$sp", WORD_SIZE, nullptr);

  /// Initialize global counter to INT_MAX and accumulator to 0
  // emit_li_instruction("$a0", 0, nullptr);
  // emit_li_instruction("$t4", INT_MAX, nullptr);

  /// Loop over the cases
  for (auto caseBinding : node->cases()) {
    /// Generate end of loop labels
    const std::string endLoop;
    const std::string mayUpdateAccumulator;

    /// Store object class identifier in register $t1
    emit_lw_instruction("$t1", "$t0", OBJECT_CLASS_OFFSET, nullptr);

    /// Store case class identifier in register $t2
    const std::string classLabel;
    emit_la_instruction("$t2", classLabel, nullptr);
    emit_lw_instruction("$t2", "$t2", OBJECT_CLASS_OFFSET, nullptr);

    /// Initialize per-class counter
    // emit_li_instruction("$t3", 0, nullptr);

    /// Start loop
    const std::string startLoop;
    emit_label(startLoop, nullptr);

    /// Go to next case statement if ancestor class not found
    // emit_bltz_instruction("$t1", endLoop, nullptr);

    /// If ancestor class found, check whether it is the closest found so far
    // emit_beq_instruction("$t1", "$t2", mayUpdateAccumulator, nullptr);

    /// Fetch next ancestor and iterate
    emit_addiu_instruction("$t3", "$t3", 0, nullptr);

    /// Jump to loop start
    emit_jump_label_instruction(startLoop, nullptr);

    /// Check whether ancestor found is the closest
    emit_label(mayUpdateAccumulator, nullptr);
    //    emit_bgt_instruction("$t3", "$t4", endLoop);

    /// Ancestor found is the closest so far, update accumulator
    emit_la_instruction("$a0", caseBinding->bindingLabel(), nullptr);

    /// End of loop label
    emit_label(endLoop, nullptr);
  }
}

/// Helper function that checks whether $a0 points to a void object and, if
/// so, interrupt execution
void TerminateExecutionIfVoid(Context *context) {
  const std::string notVoidLabel;
  emit_bgtz_instruction("$a0", notVoidLabel, nullptr);

  /// $a0 points to a void object -- interrupt program execution
  //  emit_li_instruction("$v0", 10, nullptr);
  //  emit_syscall_instruction(nullptr);

  /// Emit label for non-void instruction
  emit_label(notVoidLabel, nullptr);
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
  const std::string mnemonic = GetMnemonicFromOpType(node->opID());
  emit_lw_instruction("$t0", "$sp", WORD_SIZE, nullptr);
  emit_three_registers_instruction(mnemonic, "$a0", "$t0", "$a0", nullptr);
  push_accumulator_to_stack(nullptr);

  /// Create new integer object from proto and return pointer in $a0
  CreateObjectFromProto(context, "Int_protObj", "Int_init", nullptr);

  /// Save computed value in new integer object
  emit_lw_instruction("$t0", "$sp", WORD_SIZE, nullptr);
  emit_sw_instruction("$t0", "$a0", OBJECT_CONTENT_OFFSET, nullptr);

  /// Pop stack and return
  emit_addiu_instruction("$sp", "$sp", 4 * WORD_SIZE, nullptr);
  return Status::Ok();
}

Status CodegenPass::codegen(Context *context,
                            BinaryExprNode<ComparisonOpID> *node) {
  /// Evaluate lhs and rhs expressions
  node->lhsExpr()->generateCode(context, this);
  push_accumulator_to_stack(nullptr);
  node->rhsExpr()->generateCode(context, this);
  push_accumulator_to_stack(nullptr);

  auto funcEq = [context, node](std::iostream *ios) {
    auto *classRegistry = context->classRegistry();
    const std::string className =
        classRegistry->className(node->lhsExpr()->type().typeID);

    /// Compare values for Int and Bool objects
    if (className == "Int" || className == "Bool") {
      /// Store lhs value in $t0
      emit_lw_instruction("$t0", "$sp", 2 * WORD_SIZE, ios);
      emit_lw_instruction("$t0", "$t0", OBJECT_CONTENT_OFFSET, ios);

      /// Store rhs value in $t1
      emit_lw_instruction("$t1", "$sp", WORD_SIZE, ios);
      emit_lw_instruction("$t1", "$t1", OBJECT_CONTENT_OFFSET, ios);

      const std::string trueLabel;
      const std::string falseLabel;

      emit_compare_and_jump_instruction("beq", "$t0", "$t1", trueLabel, ios);
      CreateObjectFromProto(context, "bool_const0", "Bool_init", ios);
      emit_jump_label_instruction(falseLabel, ios);

      emit_label(trueLabel, ios);
      CreateObjectFromProto(context, "bool_const1", "Bool_init", ios);

      emit_label(falseLabel, ios);
      return;
    }

    /// Compare strings
    if (className == "String") {
      // TODO: compare strings
      return;
    }

    /// Generic case: compare the pointers
    emit_lw_instruction("$t0", "$sp", 2 * WORD_SIZE, ios);
    emit_lw_instruction("$t1", "$sp", WORD_SIZE, ios);

    const std::string trueLabel;
    const std::string falseLabel;

    emit_compare_and_jump_instruction("beq", "$t0", "$t1", trueLabel, ios);
    CreateObjectFromProto(context, "bool_const0", "Bool_init", ios);
    emit_jump_label_instruction(falseLabel, ios);

    emit_label(trueLabel, ios);
    CreateObjectFromProto(context, "bool_const1", "Bool_init", ios);

    emit_label(falseLabel, ios);
  };

  auto funcLessThanOrEq = [context, node](std::iostream *ios) {
    /// Store lhs value in $t0
    emit_lw_instruction("$t0", "$sp", 2 * WORD_SIZE, ios);
    emit_lw_instruction("$t0", "$t0", OBJECT_CONTENT_OFFSET, ios);

    /// Store rhs value in $t1
    emit_lw_instruction("$t1", "$sp", WORD_SIZE, ios);
    emit_lw_instruction("$t1", "$t1", OBJECT_CONTENT_OFFSET, ios);

    const std::string trueLabel;
    const std::string falseLabel;

    const std::string mnemonic = GetMnemonicFromOpType(node->opID());
    emit_compare_and_jump_instruction(mnemonic, "$t0", "$t1", trueLabel, ios);
    CreateObjectFromProto(context, "bool_const0", "Bool_init", ios);
    emit_jump_label_instruction(falseLabel, ios);

    emit_label(trueLabel, ios);
    CreateObjectFromProto(context, "bool_const1", "Bool_init", ios);

    emit_label(falseLabel, ios);
  };

  /// Compare values
  if (node->opID() != ComparisonOpID::Equal) {
    funcLessThanOrEq(nullptr);
  } else {
    funcEq(nullptr);
  }

  return Status::Ok();
}

Status CodegenPass::codegen(Context *context, BooleanExprNode *node) {
  const std::string protoLabel = node->value() ? "bool_const1" : "bool_const0";
  CreateObjectFromProto(context, protoLabel, "Bool_init", nullptr);
  return Status::Ok();
}

Status CodegenPass::codegen(Context *context, CaseBindingNode *node) {
  /// TODO: update environment

  /// Emit label
  emit_label(node->bindingLabel(), nullptr);

  /// Emit code for label
  node->expr()->generateCode(context, this);

  /// TODO: restore environment and return
  return Status::Ok();
}

Status CodegenPass::codegen(Context *context, CaseExprNode *node) {
  /// Evaluate case expression
  node->expr()->generateCode(context, this);
  push_accumulator_to_stack(nullptr);

  /// Interrupt execution if case expression is void
  TerminateExecutionIfVoid(context);

  /// Select case statement. Interrupt execution if no case is found
  SelectCaseStatement(context, node);
  TerminateExecutionIfVoid(context);

  /// Jump to case statement
  emit_jump_register_instruction("$a0", nullptr);

  /// Generate code for each case statement
  const std::string endLabel;
  for (auto binding : node->cases()) {
    binding->generateCode(context, this);
    emit_jump_label_instruction(endLabel, nullptr);
  }

  /// Emit end label, restore stack and return
  emit_label(endLabel, nullptr);
  emit_addiu_instruction("$sp", "$sp", WORD_SIZE, nullptr);
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
  emit_jump_label_instruction(endLabel, nullptr);

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

Status CodegenPass::codegen(Context *context, LiteralExprNode<int32_t> *node) {
  /// Create default Int object and store new value into $t0
  CreateIntObject(context, node->value(), nullptr);
  return Status::Ok();
}

Status CodegenPass::codegen(Context *context,
                            LiteralExprNode<std::string> *node) {
  /// Copy prototype object and initialize it
  const std::string label;
  CreateObjectFromProto(context, label, "String_init", nullptr);
  push_accumulator_to_stack(nullptr);

  /// Create Int value for string length
  CreateIntObject(context, node->value().length(), nullptr);
  push_accumulator_to_stack(nullptr);

  /// Update string length and return
  emit_lw_instruction("$a0", "$sp", 2 * WORD_SIZE, nullptr);
  emit_lw_instruction("$t0", "$sp", WORD_SIZE, nullptr);
  emit_sw_instruction("$t0", "$a0", STRING_LENGTH_OFFSET, nullptr);
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

Status CodegenPass::codegen(Context *context, NewExprNode *node) {
  /// Get class name
  auto registry = context->classRegistry();
  const std::string className = registry->className(node->type().typeID);

  /// Must fetch prototype from class table for self type
  if (node->type().isSelf) {
    CreateObjectFromTypeID(context, node->type().typeID, nullptr);
  } else {
    CreateObjectFromProto(context, className + "_protObj", className + "_init",
                          nullptr);
  }

  return Status::Ok();
}

Status CodegenPass::codegen(Context *context, UnaryExprNode *node) {
  /// Generate code for the unary expression
  node->expr()->generateCode(context, this);

  /// Function to generate code for IsVoid and Not
  auto funcIsVoidNot = [context](std::iostream *ios) {
    /// Generate labels
    const std::string trueLabel;
    const std::string falseLabel;

    /// Generate instructions for branch on void
    emit_beqz_instruction("$a0", trueLabel, ios);
    CreateObjectFromProto(context, "bool_const0", "Bool_init", ios);
    emit_jump_label_instruction(falseLabel, ios);

    emit_label(trueLabel, ios);
    CreateObjectFromProto(context, "bool_const1", "Bool_init", ios);

    emit_label(falseLabel, ios);
  };

  /// Function to generate code for integer complement
  auto funcComp = [context](std::iostream *ios) {
    /// Store complement of int value on the stack
    emit_lw_instruction("$a0", "$a0", OBJECT_CONTENT_OFFSET, ios);
    emit_three_registers_instruction("sub", "$a0", "$zero", "$a0", ios);
    push_accumulator_to_stack(ios);

    /// Create a new integer value and update its value
    CreateObjectFromProto(context, "Int_protObj", "Int_init", ios);
    emit_lw_instruction("$t0", "$sp", WORD_SIZE, ios);
    emit_sw_instruction("$t0", "$a0", OBJECT_CONTENT_OFFSET, ios);
  };

  /// Generate code for the three different unary operations
  if (node->opID() == UnaryOpID::IsVoid) {
    funcIsVoidNot(nullptr);
  } else if (node->opID() == UnaryOpID::Not) {
    emit_lw_instruction("$a0", "$a0", OBJECT_CONTENT_OFFSET, nullptr);
    funcIsVoidNot(nullptr);
  } else {
    funcComp(nullptr);
  }

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
  emit_jump_label_instruction(loopStartLabel, nullptr);

  /// Emit label for end of loop construct
  emit_label(loopEndLabel, nullptr);

  /// Void the return value and return
  emit_move_instruction("$a0", "$zero", nullptr);
  return Status::Ok();
}

} // namespace cool
