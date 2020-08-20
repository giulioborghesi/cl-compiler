#include <cool/codegen/codegen.h>
#include <cool/codegen/codegen_helpers.h>
#include <cool/core/context.h>
#include <cool/ir/class.h>
#include <cool/ir/expr.h>

namespace cool {

namespace {

/// Forward declaration
void CreateObjectFromProto(Context *context, const std::string &protoLabel,
                           const std::string &initLabel, std::iostream *ios);

/// Helper function to copy an object and initialize it
void CopyAndInitializeObject(Context *context, const std::string &initLabel,
                             std::iostream *ios) {
  /// Create a copy of the object and store it on the stack
  emit_jump_and_link_instruction("Object.copy", ios);
  push_accumulator_to_stack(ios);

  /// Initialize object
  emit_jump_and_link_instruction(initLabel, ios);

  /// Store address of new object in accumulator register and restore stack
  emit_lw_instruction("$a0", "$sp", WORD_SIZE, ios);
  emit_addiu_instruction("$sp", "$sp", WORD_SIZE, ios);
}

/// Helper function to create an integer object storing a given int value
///
/// \param[in] context Codegen context
/// \param[in] value value of Int object
/// \param[out] ios output stream
void CreateIntObject(Context *context, const int32_t value,
                     std::iostream *ios) {
  CreateObjectFromProto(context, "Int_protObj", "Int_init", ios);
  emit_li_instruction("$t0", value, ios);

  /// Update Int value and return
  emit_sw_instruction("$t0", "$a0", OBJECT_CONTENT_OFFSET, ios);
}

/// Helper function to create a string object storing a literal string
///
/// \param[in] context Codegen context
/// \param[in] literalProto string literal proto label
/// \param[in] stringLength string length
/// \param[out] ios output stream
void CreateStringObject(Context *context, const std::string &literalProto,
                        const size_t stringLength, std::iostream *ios) {
  /// Copy literal prototype and store it on stack
  CreateObjectFromProto(context, literalProto, "String_init", ios);
  push_accumulator_to_stack(ios);

  /// Create Int value for string length
  CreateIntObject(context, stringLength, ios);
  push_accumulator_to_stack(ios);

  /// Store string object in a0 and update string length
  emit_lw_instruction("$a0", "$sp", 2 * WORD_SIZE, ios);
  emit_lw_instruction("$t0", "$sp", WORD_SIZE, ios);
  emit_sw_instruction("$t0", "$a0", STRING_LENGTH_OFFSET, ios);

  /// Restore stack
  emit_addiu_instruction("$sp", "$sp", 2 * WORD_SIZE, ios);
}

/// Helper function to create a string object for the name of an object class
///
/// \param[in] context Codegen context
/// \param[out] ios output stream
void CreateStringObjectForClassName(Context *context, std::iostream *ios) {
  /// Store prototype object on the stack
  push_accumulator_to_stack(ios);

  /// Store prototype object address into $a0
  emit_lw_instruction("$t0", "$a0", CLASS_ID_OFFSET, ios);
  emit_la_instruction("$a0", "_Class_names_", ios);
  emit_three_registers_instruction("addu", "$a0", "$a0", "$t0", ios);
  emit_lw_instruction("$a0", "$a0", 0, ios);

  /// Copy prototype object and initialize it
  CopyAndInitializeObject(context, "String_init", ios);
  push_accumulator_to_stack(ios);

  /// Create default Int value to store string length
  CreateObjectFromProto(context, "Int_proObj", "Int_init", ios);
  push_accumulator_to_stack(ios);

  /// Store string object length in $a0
  emit_lw_instruction("$t0", "$sp", 3 * WORD_SIZE, ios);
  emit_lw_instruction("$t0", "$t0", CLASS_ID_OFFSET, ios);
  emit_la_instruction("$a0", "_Class_names_length_", ios);
  emit_three_registers_instruction("addu", "$a0", "$a0", "$t0", ios);
  emit_lw_instruction("$a0", "$a0", 0, ios);

  /// Update string length in Integer object
  emit_lw_instruction("$t0", "$sp", WORD_SIZE, ios);
  emit_sw_instruction("$a0", "$t0", OBJECT_CONTENT_OFFSET, ios);

  /// Store string object in a0 and update string length
  emit_lw_instruction("$a0", "$sp", 2 * WORD_SIZE, ios);
  emit_sw_instruction("$t0", "$a0", STRING_LENGTH_OFFSET, ios);

  /// Restore stack
  emit_addiu_instruction("$sp", "$sp", 3 * WORD_SIZE, ios);
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

  /// Copy the object and initialize it
  CopyAndInitializeObject(context, initLabel, ios);
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
  emit_jump_and_link_register_instruction("$t1", ios);

  /// Store address of new object in accumulator register and restore stack
  emit_lw_instruction("$a0", "$sp", WORD_SIZE, ios);
  emit_addiu_instruction("$sp", "$sp", WORD_SIZE, ios);
}

template <typename NodeT, typename FuncT>
Status DispatchMethodImpl(Context *context, CodegenPass *pass, NodeT *node,
                          FuncT func) {
  /// Evaluate parameters
  for (auto param : node->params()) {
    param->generateCode(context, pass);
    push_accumulator_to_stack(nullptr);
  }

  /// Evaluate expresssion
  if (node->expr() != nullptr) {
    node->expr()->generateCode(context, pass);
  } else {
    emit_lw_instruction("$a0", "$fp", 0, nullptr);
  }

  /// Fetch method table address
  func();

  /// Fetch method address
  const size_t methodPosition = 0; /// TODO: fetch method position from table
  emit_addiu_instruction("$t0", "$t0", methodPosition, nullptr);

  /// Jump and link and return
  emit_jump_and_link_instruction("$t0", nullptr);
  return Status::Ok();
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

/// Helper function that returns the length of a string. The pointer to the
/// string is expected to be stored in the accumulator register
///
/// \param[in] context Codegen context
/// \param[out] ios output stream
void GetStringLength(Context *context, std::iostream *ios) {
  emit_lw_instruction("$t0", "$a0", STRING_LENGTH_OFFSET, ios);
  emit_lw_instruction("$a0", "$t0", OBJECT_CONTENT_OFFSET, ios);
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
  /// Copy object address into $t0
  emit_move_instruction("$t0", "$a0", nullptr);

  /// Initialize global counter to INT_MAX and accumulator to void
  emit_move_instruction("$a0", "$zero", nullptr);
  emit_li_instruction("$t4", INT_MAX, nullptr);

  /// Loop over each of the cases
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
    emit_li_instruction("$t3", 0, nullptr);

    /// Start loop
    const std::string startLoop;
    emit_label(startLoop, nullptr);

    /// Go to next case statement if -1 has been reached
    emit_bltz_instruction("$t1", endLoop, nullptr);

    /// If current class is valid, check whether it is the closest found so far
    emit_compare_and_jump_instruction("beq", "$t1", "$t2", mayUpdateAccumulator,
                                      nullptr);

    /// Update counter
    emit_addiu_instruction("$t3", "$t3", 1, nullptr);

    /// Fetch ancestor class ID and jump to start of loop
    emit_la_instruction("$t5", "_Class_ancestors", nullptr);
    emit_three_registers_instruction("addu", "$t1", "$t1", "$t5", nullptr);
    emit_lw_instruction("$t1", "$t1", 0, nullptr);
    emit_jump_label_instruction(startLoop, nullptr);

    /// Check whether ancestor found is the closest
    emit_label(mayUpdateAccumulator, nullptr);
    emit_compare_and_jump_instruction("bgt", "$t3", "$t4", endLoop, nullptr);

    /// Class found is closest so far, update accumulator and global counter
    emit_move_instruction("$t4", "$t3", nullptr);
    emit_la_instruction("$a0", caseBinding->bindingLabel(), nullptr);

    /// End of loop label
    emit_label(endLoop, nullptr);
  }
}

/// Helper function that checks whether $a0 points to a void object and, if
/// so, interrupt execution
template <typename FuncT>
void TerminateExecutionIfVoid(Context *context, FuncT func,
                              std::iostream *ios) {
  /// Check whether object is void or not
  const std::string notVoidLabel;
  emit_bgtz_instruction("$a0", notVoidLabel, nullptr);

  /// Object is void
  func();

  /// Emit label for non-void instruction
  emit_label(notVoidLabel, nullptr);
}

} // namespace

Status CodegenPass::codegen(Context *context, AssignmentExprNode *node) {
  /// Evaluate right hand side expression
  node->rhsExpr()->generateCode(context, this);

  /// Update object. TODO: finalize fetching object
  auto symbolTable = context->symbolTable();
  if (true) {
    emit_lw_instruction("$t0", "$fp", 0, nullptr);
    emit_sw_instruction("$a0", "$t0", 0, nullptr);
  } else {
    emit_sw_instruction("$a0", "$fp", 0, nullptr);
  }

  /// Pop stack and return
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

  /// Restore stack and return
  emit_addiu_instruction("$sp", "$sp", 4 * WORD_SIZE, nullptr);
  return Status::Ok();
}

Status CodegenPass::codegen(Context *context, BlockExprNode *node) {
  for (auto expr : node->exprs()) {
    expr->generateCode(context, this);
  }
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

      const std::string endLabel;
      const std::string trueLabel;

      emit_compare_and_jump_instruction("beq", "$t0", "$t1", trueLabel, ios);
      CreateObjectFromProto(context, "bool_const0", "Bool_init", ios);
      emit_jump_label_instruction(endLabel, ios);

      emit_label(trueLabel, ios);
      CreateObjectFromProto(context, "bool_const1", "Bool_init", ios);

      emit_label(endLabel, ios);
      emit_addiu_instruction("$sp", "$sp", 2 * WORD_SIZE, nullptr);
      return;
    }

    /// Compare strings
    if (className == "String") {
      /// Get length of first string
      emit_lw_instruction("$a0", "$sp", 2 * WORD_SIZE, nullptr);
      GetStringLength(context, nullptr);
      push_accumulator_to_stack(nullptr);

      /// Get length of second string
      emit_lw_instruction("$a0", "$sp", 2 * WORD_SIZE, nullptr);
      GetStringLength(context, nullptr);

      /// Compare length and return false if not string
      const std::string checkEndLabel;
      const std::string sameLengthLabel;

      /// Compare lengths. If there is a length mismatch, return False
      emit_lw_instruction("$t0", "$sp", WORD_SIZE, nullptr);
      emit_compare_and_jump_instruction("beq", "$a0", "$t0", sameLengthLabel,
                                        nullptr);

      CreateObjectFromProto(context, "Bool_const0", "Bool_init", nullptr);
      emit_jump_label_instruction(checkEndLabel, nullptr);

      /// Lengths are the same. Compare each characters in the two strings
      emit_label(sameLengthLabel, nullptr);

      const std::string loopStartLabel;
      const std::string sameStringLabel;

      /// Evaluate string start addresses
      emit_lw_instruction("$t0", "$sp", 2 * WORD_SIZE, nullptr);
      emit_addiu_instruction("$t0", "$t0", STRING_CONTENT_OFFSET, nullptr);
      emit_lw_instruction("$t1", "$sp", 3 * WORD_SIZE, nullptr);
      emit_addiu_instruction("$t1", "$t1", STRING_CONTENT_OFFSET, nullptr);

      /// Evaluate string end address
      emit_lw_instruction("$t2", "$sp", WORD_SIZE, nullptr);
      emit_three_registers_instruction("addu", "$t2", "$t0", "$t2", nullptr);

      /// Compare the strings character by character
      emit_label(loopStartLabel, nullptr);
      emit_compare_and_jump_instruction("beq", "$t1", "$t2", sameStringLabel,
                                        nullptr);
      emit_lb_instruction("$t3", "$t0", 0, nullptr);
      emit_lb_instruction("$t4", "$t1", 0, nullptr);

      emit_addiu_instruction("$t0", "$t0", 1, nullptr);
      emit_addiu_instruction("$t1", "$t1", 1, nullptr);
      emit_compare_and_jump_instruction("beq", "$t3", "$t4", loopStartLabel,
                                        nullptr);

      /// Strings content differ. Return false
      CreateObjectFromProto(context, "Bool_const0", "Bool_init", nullptr);
      emit_jump_label_instruction(checkEndLabel, nullptr);

      /// Strings are the same. Return true
      emit_label(sameStringLabel, nullptr);
      CreateObjectFromProto(context, "Bool_const1", "Bool_init", nullptr);
      emit_label(checkEndLabel, nullptr);

      /// Restore stack and return
      emit_addiu_instruction("$sp", "$sp", 3 * WORD_SIZE, nullptr);
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
    emit_addiu_instruction("$sp", "$sp", 2 * WORD_SIZE, nullptr);
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
  /// Emit label
  emit_label(node->bindingLabel(), nullptr);

  /// Update object
  auto symbolTable = context->symbolTable();
  if (/*symbolTable->get(node->id()).isAttribute  /// TODO: implement*/ true) {
    emit_lw_instruction("$t0", "$fp", 0, nullptr);
    emit_sw_instruction("$a0", "$t0", 0, nullptr);
  } else {
    emit_sw_instruction("$a0", "$fp", 0, nullptr);
  }

  /// Emit code for case
  node->expr()->generateCode(context, this);

  /// Restore stack and return
  return Status::Ok();
}

Status CodegenPass::codegen(Context *context, CaseExprNode *node) {
  /// Evaluate case expression
  node->expr()->generateCode(context, this);
  push_accumulator_to_stack(nullptr);

  /// Interrupt execution if case expression is void
  auto voidExprError = [context, node]() {
    const size_t fileNameLength = 0; /// TODO: compute from context
    CreateStringObject(context, "_String_filename", fileNameLength, nullptr);
    emit_li_instruction("$t1", node->lineLoc(), nullptr);
    emit_jump_label_instruction("_case_abort2", nullptr);
  };
  TerminateExecutionIfVoid(context, voidExprError, nullptr);

  /// Select case statement
  SelectCaseStatement(context, node);
  push_accumulator_to_stack(nullptr);

  /// Interrupt execution if case not found
  auto noCaseError = [context, node]() {
    emit_lw_instruction("$a0", "$fp", 2 * WORD_SIZE, nullptr);
    CreateStringObjectForClassName(context, nullptr);
    emit_jump_label_instruction("_case_abort", nullptr);
  };
  TerminateExecutionIfVoid(context, noCaseError, nullptr);

  /// Load object for case expression into $a0 and case label into $t0
  emit_lw_instruction("$a0", "$sp", 2 * WORD_SIZE, nullptr);
  emit_lw_instruction("$t0", "$sp", WORD_SIZE, nullptr);

  /// Jump to case label
  emit_jump_register_instruction("$t0", nullptr);

  /// Generate code for each case statement
  const std::string endLabel;
  for (auto binding : node->cases()) {
    binding->generateCode(context, this);
    emit_jump_label_instruction(endLabel, nullptr);
  }

  /// Emit end label, restore stack and return
  emit_label(endLabel, nullptr);
  emit_addiu_instruction("$sp", "$sp", 2 * WORD_SIZE, nullptr);
  return Status::Ok();
}

Status CodegenPass::codegen(Context *context, DispatchExprNode *node) {
  /// Function to fetch method table address
  auto fetchMethodAddress = []() {
    emit_la_instruction("$t0", "_Class_method_table", nullptr);
    emit_lw_instruction("$t1", "$a0", OBJECT_CLASS_OFFSET, nullptr);
    emit_three_registers_instruction("addu", "$t0", "$t0", "$t1", nullptr);
  };

  return DispatchMethodImpl(context, this, node, fetchMethodAddress);
}

Status CodegenPass::codegen(Context *context, IfExprNode *node) {
  /// Generate labels
  const std::string falseLabel;
  const std::string endLabel;

  /// Emit code for if expression
  node->ifExpr()->generateCode(context, this);

  /// Load boolean value. Branch if false
  emit_lw_instruction("$t0", "$a0", BOOL_CONTENT_OFFSET, nullptr);
  emit_beqz_instruction("$t0", falseLabel, nullptr);

  /// Emit code for then expression
  node->thenExpr()->generateCode(context, this);
  emit_jump_label_instruction(endLabel, nullptr);

  /// Emit label for true branch
  emit_label(falseLabel, nullptr);

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

  return Status::Ok();
}

Status CodegenPass::codegen(Context *context, LiteralExprNode<int32_t> *node) {
  /// Create default Int object and store new value into $t0
  CreateIntObject(context, node->value(), nullptr);
  return Status::Ok();
}

Status CodegenPass::codegen(Context *context,
                            LiteralExprNode<std::string> *node) {
  const std::string label;
  CreateStringObject(context, label, node->value().length(), nullptr);
  return Status::Ok();
}

Status CodegenPass::codegen(Context *context, LetExprNode *node) {
  /// Generate code for let bindings
  auto symbolTable = context->symbolTable();
  for (auto binding : node->bindings()) {

    /// Enter new scope
    symbolTable->enterScope();

    /// Generate code for binding
    binding->generateCode(context, this);

    /// Update symbol table (TODO) and push accumulator to stack
    push_accumulator_to_stack(nullptr);
  }

  /// Generate code for main let expression
  node->expr()->generateCode(context, this);

  /// Unwind environment
  const size_t nCount = node->bindings().size();
  for (size_t iCount = 0; iCount < nCount; ++iCount) {
    symbolTable->exitScope();
  }

  /// Restore stack and return
  emit_addiu_instruction("$sp", "$sp", nCount * WORD_SIZE, nullptr);
  return Status::Ok();
}

Status CodegenPass::codegen(Context *context, MethodNode *node) {
  /// Fetch number of arguments
  const size_t nArgs = node->arguments().size();

  /// If method is a built-in method, just restore the stack and return
  if (!node->body()) {
    emit_addiu_instruction("$sp", "$sp", nArgs * WORD_SIZE, nullptr);
    return Status::Ok();
  }

  /// Fetch symbol table
  auto symbolTable = context->symbolTable();
  symbolTable->enterScope();

  /// Store accumulator, return address and frame pointer on stack
  emit_sw_instruction("$a0", "$sp", 0, nullptr);
  emit_sw_instruction("$ra", "$sp", WORD_SIZE, nullptr);
  emit_sw_instruction("$fp", "$sp", 2 * WORD_SIZE, nullptr);

  /// Update frame pointer and stack
  emit_move_instruction("$fp", "$sp", nullptr);
  emit_addiu_instruction("$sp", "$sp", -3 * WORD_SIZE, nullptr);

  /// Update environment
  for (size_t iArg = 0; iArg < nArgs; ++iArg) {
    //    symbolTable->addElement(node->arguments()[iArg]->id(), {.isAttribute =
    //    false, .position=-(iArg + 1)});
  }
  //  symbolTable->addElement("self", {.isAttribute = false, .position = 0});

  /// Generate code for method body
  node->body()->generateCode(context, this);

  /// Restore return address and frame pointer
  emit_lw_instruction("$ra", "$fp", WORD_SIZE, nullptr);
  emit_lw_instruction("$fp", "$fp", 2 * WORD_SIZE, nullptr);

  /// Restore stack and jump to return address
  emit_addiu_instruction("$sp", "$sp", (3 + nArgs) * WORD_SIZE, nullptr);
  emit_jump_register_instruction("$ra", nullptr);

  /// Restore symbol table and return
  symbolTable->exitScope();
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

Status CodegenPass::codegen(Context *context, StaticDispatchExprNode *node) {
  /// Function to fetch method table address
  auto fetchMethodAddress = [context, node]() {
    auto classRegistry = context->classRegistry();
    const size_t classID = classRegistry->typeID(node->callerClass());
    emit_la_instruction("$t0", "_Class_method_table", nullptr);
    emit_addiu_instruction("$t0", "$t0", classID, nullptr);
  };

  return DispatchMethodImpl(context, this, node, fetchMethodAddress);
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
  emit_lw_instruction("$t0", "$a0", BOOL_CONTENT_OFFSET, nullptr);
  emit_beqz_instruction("$t0", loopEndLabel, nullptr);

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
