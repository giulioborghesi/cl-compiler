#include <cool/codegen/codegen.h>
#include <cool/codegen/codegen_context.h>
#include <cool/codegen/codegen_helpers.h>
#include <cool/ir/class.h>
#include <cool/ir/expr.h>

#include <sstream>

namespace cool {

namespace {

/// \brief Forward declarations
void CreateObjectFromProto(CodegenContext *context,
                           const std::string &protoLabel,
                           const std::string &initLabel, std::iostream *ios);

void PopStack(CodegenContext *context, const size_t count, std::iostream *ios);

void PushStack(CodegenContext *context, const size_t count, std::iostream *ios);

void GetStringLength(CodegenContext *context, std::iostream *ios);

void PushAccumulatorToStack(CodegenContext *context, std::iostream *ios);

/// \brief Helper function to compare two objects of type Int or Bool
///
/// \note The objects to compare should be stored in register $a0 and
/// on top of the stack
///
/// \param[in] context Codegen context
/// \param[out] ios output stream
void CompareBoolAndIntObjects(CodegenContext *context, std::iostream *ios) {
  /// Store lhs value in $t0
  emit_lw_instruction("$t0", "$sp", WORD_SIZE, ios);
  emit_lw_instruction("$t0", "$t0", OBJECT_CONTENT_OFFSET, ios);

  /// Store rhs value in $a0
  emit_lw_instruction("$a0", "$a0", OBJECT_CONTENT_OFFSET, ios);

  /// Create labels
  const std::string endLabel = context->generateLabel("End");
  const std::string sameObjectLabel = context->generateLabel("SameObject");

  /// Compare values and generate code for false branch
  emit_compare_and_jump_instruction("beq", "$t0", "$a0", sameObjectLabel, ios);
  CreateObjectFromProto(context, "bool_const0", "Bool_init", ios);
  emit_jump_label_instruction(endLabel, ios);

  /// Generate code for true branch
  emit_label(sameObjectLabel, ios);
  CreateObjectFromProto(context, "bool_const1", "Bool_init", ios);

  /// Emit label
  emit_label(endLabel, ios);
}

/// \brief Helper function to compare two objects
///
/// \note The objects to compare should be stored in register $a0 and
/// on top of the stack
///
/// \param[in] context Codegen context
/// \param[out] ios output stream
void CompareObjects(CodegenContext *context, std::iostream *ios) {
  /// Store lhs object address in register $t0
  emit_lw_instruction("$t0", "$sp", WORD_SIZE, ios);

  /// Create label
  const std::string endLabel = context->generateLabel("End");
  const std::string sameObjectLabel = context->generateLabel("SameObject");

  /// Compare values and generate code for false branch
  emit_compare_and_jump_instruction("beq", "$t0", "$a0", sameObjectLabel, ios);
  CreateObjectFromProto(context, "bool_const0", "Bool_init", ios);
  emit_jump_label_instruction(endLabel, ios);

  /// Generate code for true branch
  emit_label(sameObjectLabel, ios);
  CreateObjectFromProto(context, "bool_const1", "Bool_init", ios);

  /// Emit label
  emit_label(endLabel, ios);
}

/// \brief Helper function to compare two String objects
///
/// \note The objects to compare should be stored in register $a0 and
/// on top of the stack
///
/// \param[in] context Codegen context
/// \param[out] ios output stream
void CompareStringObjects(CodegenContext *context, std::iostream *ios) {
  /// Store arguments in register $a0 on the stack
  PushAccumulatorToStack(context, ios);

  /// Get length of first string and store it in register $a0
  emit_lw_instruction("$a0", "$sp", 2 * WORD_SIZE, ios);
  GetStringLength(context, ios);
  PushAccumulatorToStack(context, ios);

  /// Get length of second string and store it in register $a0
  emit_lw_instruction("$a0", "$sp", 2 * WORD_SIZE, ios);
  GetStringLength(context, ios);

  /// Compare length and return false if not string
  const std::string checkEndLabel;
  const std::string sameLengthLabel;

  /// Compare string lengths
  emit_lw_instruction("$t0", "$sp", WORD_SIZE, ios);
  emit_compare_and_jump_instruction("beq", "$a0", "$t0", sameLengthLabel, ios);

  /// Generate code for strings of different length
  CreateObjectFromProto(context, "Bool_const0", "Bool_init", ios);
  emit_jump_label_instruction(checkEndLabel, ios);

  /// Lengths are the same. Compare each characters in the two strings
  emit_label(sameLengthLabel, ios);

  const std::string loopStartLabel;
  const std::string sameStringLabel;

  /// Store string start addresses in registers $t0 and $t1
  emit_lw_instruction("$t0", "$sp", 2 * WORD_SIZE, ios);
  emit_addiu_instruction("$t0", "$t0", STRING_CONTENT_OFFSET, ios);
  emit_lw_instruction("$t1", "$sp", 3 * WORD_SIZE, ios);
  emit_addiu_instruction("$t1", "$t1", STRING_CONTENT_OFFSET, ios);

  /// Evaluate end address of first string and store it in register $t2
  emit_lw_instruction("$t2", "$sp", WORD_SIZE, ios);
  emit_three_registers_instruction("addu", "$t2", "$t1", "$t2", ios);

  /// Compare the strings character by character
  emit_label(loopStartLabel, ios);
  emit_compare_and_jump_instruction("beq", "$t1", "$t2", sameStringLabel, ios);

  /// Load characters to compare in registers $t3 and $t4
  emit_lb_instruction("$t3", "$t0", 0, ios);
  emit_lb_instruction("$t4", "$t1", 0, ios);

  /// Advance raw string pointers
  emit_addiu_instruction("$t0", "$t0", 1, ios);
  emit_addiu_instruction("$t1", "$t1", 1, ios);

  /// Go to next characters if the current ones are the same
  emit_compare_and_jump_instruction("beq", "$t3", "$t4", loopStartLabel, ios);

  /// Characters differ. Create a Bool False object
  CreateObjectFromProto(context, "Bool_const0", "Bool_init", ios);
  emit_jump_label_instruction(checkEndLabel, ios);

  /// Strings are the same. Create a Bool True object
  emit_label(sameStringLabel, ios);
  CreateObjectFromProto(context, "Bool_const1", "Bool_init", ios);

  /// Emit label for end of check
  emit_label(checkEndLabel, ios);

  /// Restore stack
  emit_addiu_instruction("$sp", "$sp", 2 * WORD_SIZE, ios);
}

/// \brief Helper function to compute the method table key associated with a
/// method
///
/// \note The key is computed as follows: MethodName__Arg1Type_ArgNType
///
/// \param[in] node Method node
/// \return the key associated with the method
std::string ComputeMethodKey(MethodNodePtr node) {
  std::stringstream key;
  key << node->id() << "_";
  for (auto argument : node->arguments()) {
    key << "_" << argument->id();
  }
  return key.str();
}

/// \brief Helper function to copy an object and initialize it
///
/// \note The object to copy should be stored in register $a0
///
/// \param[in] context Codegen context
/// \param[in] initLabel label to initialization code
/// \param[out] ios output stream
void CopyAndInitializeObject(CodegenContext *context,
                             const std::string &initLabel, std::iostream *ios) {
  /// Create a copy of the object and store it on the stack
  emit_jump_and_link_instruction("Object.copy", ios);

  /// Initialize object
  emit_jump_and_link_instruction(initLabel, ios);
}

/// \brief Helper function to create an integer object storing a given int value
///
/// \param[in] context Codegen context
/// \param[in] value value of Int object
/// \param[out] ios output stream
void CreateIntObject(CodegenContext *context, const int32_t value,
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
void CreateStringObject(CodegenContext *context,
                        const std::string &literalProto,
                        const size_t stringLength, std::iostream *ios) {
  /// Copy literal prototype and store it on stack
  CreateObjectFromProto(context, literalProto, "String_init", ios);
  PushAccumulatorToStack(context, ios);

  /// Create Int value for string length and store it into $t0
  CreateIntObject(context, stringLength, ios);
  emit_move_instruction("$t0", "$a0", ios);

  /// Store string object in a0 and update string length
  emit_lw_instruction("$a0", "$sp", WORD_SIZE, ios);
  emit_sw_instruction("$t0", "$a0", STRING_LENGTH_OFFSET, ios);

  /// Restore stack
  emit_addiu_instruction("$sp", "$sp", WORD_SIZE, ios);
}

/// Helper function to create a string object for the name of an object class
///
/// \param[in] context Codegen context
/// \param[out] ios output stream
void CreateStringObjectForClassName(CodegenContext *context,
                                    std::iostream *ios) {
  /// Store prototype object on the stack
  PushAccumulatorToStack(context, ios);

  /// Store prototype object address into $a0
  emit_lw_instruction("$t0", "$a0", CLASS_ID_OFFSET, ios);
  emit_la_instruction("$a0", "_Class_names_", ios);
  emit_three_registers_instruction("addu", "$a0", "$a0", "$t0", ios);
  emit_lw_instruction("$a0", "$a0", 0, ios);

  /// Copy prototype object and initialize it
  CopyAndInitializeObject(context, "String_init", ios);
  PushAccumulatorToStack(context, ios);

  /// Create default Int value to store string length
  CreateObjectFromProto(context, "Int_proObj", "Int_init", ios);
  PushAccumulatorToStack(context, ios);

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
void CreateObjectFromProto(CodegenContext *context,
                           const std::string &protoLabel,
                           const std::string &initLabel, std::iostream *ios) {
  /// Load address of prototype object into $a0
  emit_la_instruction("$a0", protoLabel, ios);

  /// Copy the object and initialize it
  CopyAndInitializeObject(context, initLabel, ios);
}

/// \brief Helper function to create a copy of an object given its unique
/// class identifyer. The pointer to the newly created object is stored in
/// register $a0
///
/// \note The class ID of the object to be initialized is expected to be stored
/// in register $a0
///
/// \param[in] context Codegen context
/// \param[out] ios output stream
void CreateObjectFromTypeID(CodegenContext *context, std::iostream *ios) {
  /// Compute offsets in prototype and init tables
  emit_sll_instruction("$a0", "$a0", 2, ios);

  /// Load address of prototype object into $a0
  emit_la_instruction("$t0", "_Class_protObj_table", ios);
  emit_three_registers_instruction("addu", "$a0", "$a0", "$t0", ios);
  emit_lw_instruction("$a0", "$a0", 0, ios);

  /// Create a copy of the prototype object
  emit_jump_and_link_instruction("Object.copy", ios);

  /// Load address of init function into $t0 and initialize object
  emit_la_instruction("$t0", "_Class_init_table", ios);
  emit_three_registers_instruction("addu", "$t0", "$t0", "$t1", ios);
  emit_jump_and_link_register_instruction("$t0", ios);
}

/// \brief Pop the stack size by a specified number of elements
///
/// \param[in] context Codegen context
/// \param[in] count number of elements to pop from stack
/// \param[out] ios output stream
void PopStack(CodegenContext *context, const size_t count, std::iostream *ios) {
  emit_addiu_instruction("$sp", "$sp", count * WORD_SIZE, ios);
  context->decrementStackSize(count);
}

/// \brief Push the stack size by a specified number of elements
///
/// \param[in] context Codegen context
/// \param[in] count number of elements to push to stack
/// \param[out] ios output stream
void PushStack(CodegenContext *context, const size_t count,
               std::iostream *ios) {
  emit_addiu_instruction("$sp", "$sp", -count * WORD_SIZE, ios);
  context->incrementStackSize(count);
}

/// \brief Helper function that generate the code needed for method dispatch.
/// Code to fetch the method table address is specific to the dispatch type and
/// is generated by a user-passed lambda function or functor
///
/// \note The function that generates the code for fetching the method table
/// address should not modify the content of register $a0
///
/// \param[in] context Codegen context
/// \param[in] pass Codegen pass
/// \param[in] node dispatch expression node
/// \param[in] fetchTableAddress unction to fetch method table address
/// dispatch-specific code \param[out] ios output stream
template <typename NodeT, typename FuncT>
Status GenerateDispatchCode(CodegenContext *context, CodegenPass *pass,
                            NodeT *node, FuncT fetchTableAddress,
                            std::iostream *ios) {
  /// Evaluate parameters
  for (auto param : node->params()) {
    param->generateCode(context, pass, ios);
    PushAccumulatorToStack(context, ios);
  }

  /// Evaluate expresssion if applicable, otherwise fetch self object
  if (node->expr() != nullptr) {
    node->expr()->generateCode(context, pass, ios);
  } else {
    emit_lw_instruction("$a0", "$fp", 0, ios);
  }

  /// Fetch method table address
  fetchTableAddress();

  /// Fetch method address
  const auto methodInfo = context->methodTable()->get(node->methodName());
  const size_t methodPosition = methodInfo.position;
  emit_addiu_instruction("$t0", "$t0", methodPosition, ios);

  /// Transfer control to function and return
  emit_jump_and_link_instruction("$t0", ios);
  return Status::Ok();
}

/// \brief Helper function to get the mnemonic corresponding to a given
/// arithmetic operator
///
/// \param[in] opID arithmetic operator ID
/// \return the mnemonic corresponding to the given arithmetic operator ID
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
  assert(0);
}

/// \brief Helper function to get the mnemonic corresponding to a given
/// comparison operator
///
/// \param[in] opID comparison operator ID
/// \return the mnemonic corresponding to the given comparison operator ID
std::string GetMnemonicFromOpType(const ComparisonOpID opID) {
  return opID == ComparisonOpID::LessThan ? "blt" : "ble";
}

/// Helper function that returns the length of a string. The pointer to the
/// string is expected to be stored in the accumulator register
///
/// \param[in] context Codegen context
/// \param[out] ios output stream
void GetStringLength(CodegenContext *context, std::iostream *ios) {
  emit_lw_instruction("$t0", "$a0", STRING_LENGTH_OFFSET, ios);
  emit_lw_instruction("$a0", "$t0", OBJECT_CONTENT_OFFSET, ios);
}

/// \brief Emit a sequence of MIPS instruction to push the accumulator to stack
/// and update the stack pointer. Also increase the stack size counter in the
/// codegen context
///
/// \param[in] context Codegen context
/// \param[out] ios output stream
void PushAccumulatorToStack(CodegenContext *context, std::iostream *ios) {
  emit_sw_instruction("$ra", "$sp", 0, ios);
  emit_addiu_instruction("$sp", "$sp", -4, ios);
  context->incrementStackSize(1);
}

/// \brief Helper function that determine the case statement to take and store
/// its address in register $a0
//
/// \param[in] context Codegen context
/// \param[in] node Case expression node
/// \param[out] ios output stream
void SelectCaseStatement(CodegenContext *context, CaseExprNode *node,
                         std::iostream *ios) {
  /// Fetch class registry
  auto registry = context->classRegistry();

  /// Copy object address into $t0
  emit_move_instruction("$t0", "$a0", ios);

  /// Initialize global counter to INT_MAX and accumulator to void
  emit_move_instruction("$a0", "$0", ios);
  emit_li_instruction("$t4", INT_MAX, ios);

  /// Loop over the cases in the case expression node
  for (auto caseBinding : node->cases()) {
    /// Create labels
    const std::string endLabel = context->generateLabel("End");
    const std::string updateLabel = context->generateLabel("UpdateCase");

    /// Store current class identifier in register $t1
    emit_lw_instruction("$t1", "$t0", CLASS_ID_OFFSET, ios);

    /// Store case class identifier in register $t2
    const size_t classID = registry->typeID(caseBinding->id());
    emit_li_instruction("$t2", classID, ios);

    /// Initialize local counter
    emit_li_instruction("$t3", 0, ios);

    /// Loop until a match is found
    const std::string startLoop = context->generateLabel("Begin");
    emit_label(startLoop, ios);

    /// Go to next case statement if -1 has been reached
    emit_bltz_instruction("$t1", endLabel, ios);

    /// If current class is valid, check whether it is the closest found so far
    emit_compare_and_jump_instruction("beq", "$t1", "$t2", updateLabel, ios);

    /// Update local counter
    emit_addiu_instruction("$t3", "$t3", 1, ios);

    /// Fetch ancestor class ID and jump to start of loop
    emit_sll_instruction("$t1", "$t1", 2, ios);
    emit_la_instruction("$t5", "_Class_ancestor", ios);
    emit_three_registers_instruction("addu", "$t1", "$t1", "$t5", ios);
    emit_lw_instruction("$t1", "$t1", 0, ios);

    /// Repeat check on ancestor class
    emit_jump_label_instruction(startLoop, ios);

    /// Ancesto found. Check whether found ancestor is the closest
    emit_label(updateLabel, ios);
    emit_compare_and_jump_instruction("bgt", "$t3", "$t4", endLabel, ios);

    /// Ancestor found is the closest, update result and global counter
    emit_move_instruction("$t4", "$t3", ios);
    emit_la_instruction("$a0", caseBinding->bindingLabel(), ios);

    /// End of loop label
    emit_label(endLabel, ios);
  }
}

/// \brief Helper function that checks whether $a0 points to a void object and,
/// if so, interrupt execution
///
/// \param[in] context Codegen context
/// \param[in] errorFunc Functor / lambda to generate error handling code
/// \param[out] ios output stream
template <typename FuncT>
void TerminateExecutionIfVoid(CodegenContext *context, FuncT errorFunc,
                              std::iostream *ios) {
  /// Check whether object is void or not
  const std::string notVoidLabel = context->generateLabel("NotVoid");
  emit_bgtz_instruction("$a0", notVoidLabel, ios);

  /// Object is void. Generate code to handle error
  errorFunc();

  /// Emit label for non-void instruction
  emit_label(notVoidLabel, ios);
}

} // namespace

/// DONE
Status CodegenPass::codegen(CodegenContext *context, AssignmentExprNode *node,
                            std::iostream *ios) {
  /// Generate code for right hand side expression
  node->rhsExpr()->generateCode(context, this, ios);

  /// Update object
  auto symbolInfo = context->symbolTable()->get(node->id());
  const bool isAttribute = symbolInfo.isAttribute;
  const size_t variablePosition = symbolInfo.position;
  if (isAttribute) {
    emit_lw_instruction("$t0", "$fp", 0, ios);
    emit_sw_instruction("$a0", "$t0", variablePosition * WORD_SIZE, ios);
  } else {
    emit_sw_instruction("$a0", "$fp", variablePosition * WORD_SIZE, ios);
  }

  /// All good, stack did not change
  return Status::Ok();
}

/// DONE
Status CodegenPass::codegen(CodegenContext *context, AttributeNode *node,
                            std::iostream *ios) {
  /// Fetch symbol table
  auto symbolTable = context->symbolTable();

  /// Generate attribute initialization code
  node->initExpr()->generateCode(context, this, ios);
  emit_move_instruction("$t0", "$a0", ios);

  /// Update attribute
  auto symbolInfo = context->symbolTable()->get(node->id());
  const size_t attrPosition = symbolInfo.position;
  const size_t offset = attrPosition * WORD_SIZE + OBJECT_CONTENT_OFFSET;
  emit_lw_instruction("$a0", "$fp", 0, ios);
  emit_sw_instruction("$t0", "$a0", offset, ios);

  /// Return
  return Status::Ok();
}

/// DONE
Status CodegenPass::codegen(CodegenContext *context,
                            BinaryExprNode<ArithmeticOpID> *node,
                            std::iostream *ios) {
  /// Evaluate left and right hand side expressions
  node->lhsExpr()->generateCode(context, this, ios);
  PushAccumulatorToStack(context, ios);
  node->rhsExpr()->generateCode(context, this, ios);

  /// Store lhs value on register $t0
  emit_lw_instruction("$t0", "$sp", WORD_SIZE, ios);
  emit_lw_instruction("$t0", "$t0", OBJECT_CONTENT_OFFSET, ios);

  /// Store rhs value in register $a0
  emit_lw_instruction("$a0", "$a0", OBJECT_CONTENT_OFFSET, ios);

  /// Sum values and store result in register $a0
  const std::string mnemonic = GetMnemonicFromOpType(node->opID());
  emit_three_registers_instruction(mnemonic, "$a0", "$a0", "$t0", ios);
  PushAccumulatorToStack(context, ios);

  /// Create new integer object from proto and return pointer in $a0
  CreateObjectFromProto(context, "Int_protObj", "Int_init", ios);

  /// Save computed value in new integer object
  emit_lw_instruction("$t0", "$sp", WORD_SIZE, ios);
  emit_sw_instruction("$t0", "$a0", OBJECT_CONTENT_OFFSET, ios);

  /// Restore stack and return
  PopStack(context, 2, ios);
  return Status::Ok();
}

/// DONE
Status CodegenPass::codegen(CodegenContext *context,
                            BinaryExprNode<ComparisonOpID> *node,
                            std::iostream *ios) {
  /// Evaluate lhs and rhs expressions
  node->lhsExpr()->generateCode(context, this, ios);
  PushAccumulatorToStack(context, ios);
  node->rhsExpr()->generateCode(context, this, ios);

  auto funcLessThanOrEq = [context, node, ios]() {
    /// Store lhs value in $t0
    emit_lw_instruction("$t0", "$sp", WORD_SIZE, ios);
    emit_lw_instruction("$t0", "$t0", OBJECT_CONTENT_OFFSET, ios);

    /// Store rhs value in $t1
    emit_lw_instruction("$t1", "$a0", OBJECT_CONTENT_OFFSET, ios);

    /// End label
    const std::string endLabel = context->generateLabel("End");
    const std::string trueLabel = context->generateLabel("True");

    /// Compare values
    const std::string mnemonic = GetMnemonicFromOpType(node->opID());
    emit_compare_and_jump_instruction(mnemonic, "$t0", "$t1", trueLabel, ios);

    /// Inequality not satisfied. Create Bool False object
    CreateObjectFromProto(context, "bool_const0", "Bool_init", ios);
    emit_jump_label_instruction(endLabel, ios);

    /// Inequality satisfied. Create Bool True object
    emit_label(trueLabel, ios);
    CreateObjectFromProto(context, "bool_const1", "Bool_init", ios);

    /// Emit end of comparison construct label
    emit_label(endLabel, ios);
  };

  auto funcCompareObjects = [context, node, ios]() {
    /// Get lhs object type name
    auto registry = context->classRegistry();
    const std::string typeName =
        registry->className(node->lhsExpr()->type().typeID);

    /// Take decision based on object type
    if (typeName == "Int" || typeName == "Bool") {
      CompareBoolAndIntObjects(context, ios);
    } else if (typeName == "String") {
      CompareStringObjects(context, ios);
    } else {
      CompareObjects(context, ios);
    }
  };

  /// Compare values
  if (node->opID() != ComparisonOpID::Equal) {
    funcLessThanOrEq();
  } else {
    funcCompareObjects();
  }

  /// Restore stack and return
  PopStack(context, 1, ios);
  return Status::Ok();
}

/// DONE
Status CodegenPass::codegen(CodegenContext *context, BlockExprNode *node,
                            std::iostream *ios) {
  for (auto expr : node->exprs()) {
    expr->generateCode(context, this, ios);
  }
  return Status::Ok();
}

/// DONE
Status CodegenPass::codegen(CodegenContext *context, BooleanExprNode *node,
                            std::iostream *ios) {
  const std::string protoLabel = node->value() ? "bool_const1" : "bool_const0";
  CreateObjectFromProto(context, protoLabel, "Bool_init", ios);
  return Status::Ok();
}

/// DONE
Status CodegenPass::codegen(CodegenContext *context, CaseBindingNode *node,
                            std::iostream *ios) {
  /// Emit label
  const std::string bindingLabel =
      context->generateLabel("Binding_" + node->id());
  emit_label(bindingLabel, ios);

  /// Enter a new symbol table scope
  auto symbolTable = context->symbolTable();
  symbolTable->enterScope();

  /// Update symbol table. Value is stored one position below top of stack
  const size_t position = context->stackSize() - 1;
  symbolTable->addElement(node->id(), IdentifierCodegenInfo(false, position));

  /// Emit code for case binding
  node->expr()->generateCode(context, this, ios);

  /// Restore the symbol table scope and return
  symbolTable->exitScope();
  return Status::Ok();
}

/// DONE
Status CodegenPass::codegen(CodegenContext *context, CaseExprNode *node,
                            std::iostream *ios) {
  /// Evaluate case expression
  node->expr()->generateCode(context, this, ios);
  PushAccumulatorToStack(context, ios);

  /// Interrupt execution if case expression is void
  auto voidExprError = [context, node, ios]() {
    const size_t fileNameLength = 0; /// TODO: compute from context
    CreateStringObject(context, "_Program_filename", fileNameLength, ios);
    emit_li_instruction("$t1", node->lineLoc(), ios);
    emit_jump_label_instruction("_case_abort2", ios);
  };
  TerminateExecutionIfVoid(context, voidExprError, ios);

  /// Select case statement
  SelectCaseStatement(context, node, ios);
  PushAccumulatorToStack(context, ios);

  /// Interrupt execution if case not found
  auto noCaseError = [context, node, ios]() {
    emit_lw_instruction("$a0", "$sp", 2 * WORD_SIZE, ios);
    CreateStringObjectForClassName(context, ios);
    emit_jump_label_instruction("_case_abort", ios);
  };
  TerminateExecutionIfVoid(context, noCaseError, ios);

  /// Load object for case expression into $a0 and case label into $t0
  emit_lw_instruction("$a0", "$sp", 2 * WORD_SIZE, ios);
  emit_lw_instruction("$t0", "$sp", 1 * WORD_SIZE, ios);

  /// Jump to case label
  emit_jump_register_instruction("$t0", ios);

  /// Generate code for each case statement
  const std::string endLabel = context->generateLabel("End");
  for (auto binding : node->cases()) {
    binding->generateCode(context, this, ios);
    emit_jump_label_instruction(endLabel, ios);
  }

  /// Emit end label, restore stack and return
  emit_label(endLabel, ios);
  PopStack(context, 2, ios);
  return Status::Ok();
}

/// DONE
Status CodegenPass::codegen(CodegenContext *context, ClassNode *node,
                            std::iostream *ios) {
  /// Initialize stack, symbol table and method table
  context->resetStackSize();
  context->setCurrentClassName(node->className());
  context->initializeTables();

  /// Initialize the location of each attribute
  auto symbolTable = context->symbolTable();
  for (auto attribute : node->attributes()) {
    const size_t attrPosition = symbolTable->count();
    IdentifierCodegenInfo attrInfo(true, attrPosition);
    symbolTable->addElement(attribute->id(), attrInfo);
  }

  /// Initialize the location of each method
  auto methodTable = context->methodTable();
  for (auto method : node->methods()) {
    const std::string methodKey = ComputeMethodKey(method);
    if (!methodTable->findKeyInTable(methodKey)) {
      const size_t methodPosition = methodTable->count();
      MethodCodegenInfo methodInfo(methodPosition);
    }
  }

  /// Emit class initialization label
  emit_label(node->className() + "_init", ios);

  /// Store accumulator, return address and frame pointer on stack
  emit_sw_instruction("$a0", "$sp", 0 * WORD_SIZE, ios);
  emit_sw_instruction("$ra", "$sp", 1 * WORD_SIZE, ios);
  emit_sw_instruction("$fp", "$sp", 2 * WORD_SIZE, ios);

  /// Update stack and frame pointer
  emit_move_instruction("$fp", "$sp", ios);
  PushStack(context, 3, ios);

  /// Initialize parent class if applicable
  if (node->hasParentClass()) {
    const std::string jumpLabel = node->parentClassName() + "_init";
    emit_jump_and_link_instruction(jumpLabel, ios);
  }

  /// Lambda function to generate a default object for the built-in classes
  auto generateDefaultObject = [context, ios](const std::string &typeName) {
    if (typeName == "Int") {
      CreateIntObject(context, 0, ios);
    } else if (typeName == "Bool") {
      CreateObjectFromProto(context, "Bool_const0", "Bool_init", ios);
    } else if (typeName == "String") {
      CreateStringObject(context, "String_protObj", 0, ios);
    }
  };

  /// Initialize String, Bool and Int objects to their default values
  for (auto attribute : node->attributes()) {
    const std::string typeName = attribute->typeName();
    if (typeName == "String" || typeName == "Int" || typeName == "Bool") {
      /// Generate default object for attribute
      generateDefaultObject(typeName);

      /// Update attribute
      const size_t attrPosition = symbolTable->get(attribute->id()).position;
      const size_t offset = attrPosition * WORD_SIZE + OBJECT_CONTENT_OFFSET;
      emit_lw_instruction("$t0", "$fp", 0, ios);
      emit_sw_instruction("$a0", "$t0", offset, ios);
    }
  }

  /// Store self object in $a0
  emit_lw_instruction("$a0", "$fp", 0, ios);

  /// Generate code for attributes initialization
  for (auto attribute : node->attributes()) {
    attribute->generateCode(context, this, ios);
  }

  /// Restore return address, frame pointer and self object
  emit_lw_instruction("$a0", "$fp", 0 * WORD_SIZE, ios);
  emit_lw_instruction("$ra", "$fp", 1 * WORD_SIZE, ios);
  emit_lw_instruction("$fp", "$fp", 2 * WORD_SIZE, ios);

  /// Finalize init by restoring stack and jump to return register
  PopStack(context, 3, ios);
  emit_jump_register_instruction("$ra", ios);

  /// Generate code for class methods and return
  for (auto method : node->methods()) {
    method->generateCode(context, this, ios);
  }

  return Status::Ok();
}

/// DONE
Status CodegenPass::codegen(CodegenContext *context, DispatchExprNode *node,
                            std::iostream *ios) {
  /// Function to fetch method table address
  auto fetchMethodAddress = [ios]() {
    emit_la_instruction("$t0", "_Class_method_table", ios);
    emit_lw_instruction("$t1", "$a0", CLASS_ID_OFFSET, ios);
    emit_sll_instruction("$t1", "$t1", 2, ios);
    emit_three_registers_instruction("addu", "$t0", "$t0", "$t1", ios);
  };

  return GenerateDispatchCode(context, this, node, fetchMethodAddress, ios);
}

/// DONE
Status CodegenPass::codegen(CodegenContext *context, IdExprNode *node,
                            std::iostream *ios) {
  if (node->id() == "self") {
    emit_lw_instruction("$a0", "$fp", 0, ios);
  } else {
    const auto variableInfo = context->symbolTable()->get(node->id());
    const int32_t position = variableInfo.position;
    if (variableInfo.isAttribute) {
      const int32_t offset = (position + OBJECT_CONTENT_OFFSET) * WORD_SIZE;
      emit_lw_instruction("$a0", "$fp", 0, ios);
      emit_lw_instruction("$a0", "$a0", offset, ios);
    } else {
      emit_lw_instruction("$a0", "$fp", position * WORD_SIZE, ios);
    }
  }
  return Status::Ok();
}

/// DONE
Status CodegenPass::codegen(CodegenContext *context, IfExprNode *node,
                            std::iostream *ios) {
  /// Create labels
  const std::string falseLabel = context->generateLabel("False");
  const std::string endLabel = context->generateLabel("End");

  /// Emit code for if expression
  node->ifExpr()->generateCode(context, this, ios);

  /// Load boolean value. Branch if false
  emit_lw_instruction("$a0", "$a0", BOOL_CONTENT_OFFSET, ios);
  emit_beqz_instruction("$a0", falseLabel, ios);

  /// Emit code for then expression
  node->thenExpr()->generateCode(context, this, ios);
  emit_jump_label_instruction(endLabel, ios);

  /// Emit label for true branch
  emit_label(falseLabel, ios);

  /// Emit code for else expression
  node->elseExpr()->generateCode(context, this, ios);

  /// Emit label for end of if construct and return
  emit_label(endLabel, ios);
  return Status::Ok();
}

/// DONE
Status CodegenPass::codegen(CodegenContext *context,
                            LiteralExprNode<int32_t> *node,
                            std::iostream *ios) {
  CreateIntObject(context, node->value(), ios);
  return Status::Ok();
}

/// TODO: generate string proto
Status CodegenPass::codegen(CodegenContext *context,
                            LiteralExprNode<std::string> *node,
                            std::iostream *ios) {
  const std::string stringProto;
  CreateStringObject(context, stringProto, node->value().length(), ios);
  return Status::Ok();
}

/// DONE
Status CodegenPass::codegen(CodegenContext *context, LetBindingNode *node,
                            std::iostream *ios) {
  /// Generate code for right hand side expression
  if (node->hasExpr()) {
    node->expr()->generateCode(context, this, ios);
  } else {
    const std::string typeName = node->typeName();
    if (typeName == "SELF_TYPE") {
      emit_lw_instruction("$a0", "$fp", 0, ios);
      emit_lw_instruction("$a0", "$a0", CLASS_ID_OFFSET, ios);
      CreateObjectFromTypeID(context, ios);
    } else {
      CreateObjectFromProto(context, typeName + "_protObj", typeName + "_init",
                            ios);
    }
  }

  return Status::Ok();
}

/// DONE
Status CodegenPass::codegen(CodegenContext *context, LetExprNode *node,
                            std::iostream *ios) {
  /// Fetch symbol table
  auto symbolTable = context->symbolTable();

  /// Generate code for let bindings
  for (auto binding : node->bindings()) {

    /// Enter new scope
    symbolTable->enterScope();

    /// Generate code for binding
    binding->generateCode(context, this, ios);

    /// Update symbol table and push accumulator to stack
    const size_t position = context->stackSize();
    symbolTable->addElement(binding->id(),
                            IdentifierCodegenInfo(false, position));
    PushAccumulatorToStack(context, ios);
  }

  /// Generate code for main let expression
  node->expr()->generateCode(context, this, ios);

  /// Unwind environment
  const size_t nCount = node->bindings().size();
  for (size_t iCount = 0; iCount < nCount; ++iCount) {
    symbolTable->exitScope();
  }

  /// Restore stack and return
  PopStack(context, nCount, ios);
  return Status::Ok();
}

/// DONE
Status CodegenPass::codegen(CodegenContext *context, MethodNode *node,
                            std::iostream *ios) {
  /// Nothing to do for built-in methods
  if (!node->body()) {
    return Status::Ok();
  }

  /// Store number of arguments in local variable
  const size_t nArgs = node->arguments().size();

  /// Reset stack size, fetch symbol table and enter a new scope
  context->resetStackSize();
  auto symbolTable = context->symbolTable();
  symbolTable->enterScope();

  /// Emit method label
  emit_label(context->currentClassName() + "." + node->id(), ios);

  /// Store accumulator, return address and frame pointer on stack
  emit_sw_instruction("$a0", "$sp", 0 * WORD_SIZE, ios);
  emit_sw_instruction("$ra", "$sp", 1 * WORD_SIZE, ios);
  emit_sw_instruction("$fp", "$sp", 2 * WORD_SIZE, ios);

  /// Update frame pointer and stack
  emit_move_instruction("$fp", "$sp", ios);
  PushStack(context, 3, ios);

  /// Update environment
  for (size_t iArg = 0; iArg < nArgs; ++iArg) {
    IdentifierCodegenInfo argInfo(false, -(nArgs - iArg - 1));
    symbolTable->addElement(node->arguments()[iArg]->id(), argInfo);
  }
  symbolTable->addElement("self", IdentifierCodegenInfo(false, 0));

  /// Generate code for method body
  node->body()->generateCode(context, this, ios);

  /// Restore return address and frame pointer
  emit_lw_instruction("$ra", "$fp", 1 * WORD_SIZE, ios);
  emit_lw_instruction("$fp", "$fp", 2 * WORD_SIZE, ios);

  /// Restore stack and jump to return address
  PopStack(context, 3 + nArgs, ios);
  emit_jump_register_instruction("$ra", ios);

  /// Exit scope and return
  symbolTable->exitScope();
  return Status::Ok();
}

/// DONE
Status CodegenPass::codegen(CodegenContext *context, NewExprNode *node,
                            std::iostream *ios) {
  /// Get class name
  auto registry = context->classRegistry();
  const std::string className = registry->className(node->type().typeID);

  /// Create new object. Self object computes methods addresses from class ID
  if (node->type().isSelf) {
    emit_lw_instruction("$a0", "$fp", 0, ios);
    emit_lw_instruction("$a0", "$a0", CLASS_ID_OFFSET, ios);
    CreateObjectFromTypeID(context, ios);
  } else {
    CreateObjectFromProto(context, className + "_protObj", className + "_init",
                          ios);
  }
  return Status::Ok();
}

/// DONE
Status CodegenPass::codegen(CodegenContext *context, ProgramNode *node,
                            std::iostream *ios) {
  for (auto classNode : node->classes()) {
    classNode->generateCode(context, this, ios);
  }
  return Status::Ok();
}

/// DONE
Status CodegenPass::codegen(CodegenContext *context,
                            StaticDispatchExprNode *node, std::iostream *ios) {
  /// Function to fetch method table address
  auto fetchTableAddress = [context, node, ios]() {
    auto classRegistry = context->classRegistry();
    const size_t classID = classRegistry->typeID(node->callerClass());
    emit_la_instruction("$t0", "_Class_method_table", ios);
    emit_addiu_instruction("$t0", "$t0", classID * WORD_SIZE, ios);
  };

  return GenerateDispatchCode(context, this, node, fetchTableAddress, ios);
}

/// DONE
Status CodegenPass::codegen(CodegenContext *context, UnaryExprNode *node,
                            std::iostream *ios) {
  /// Generate code for the unary expression
  node->expr()->generateCode(context, this, ios);

  /// Function to generate code for IsVoid and Not
  auto funcIsVoidAndNot = [context](std::iostream *ios) {
    /// Generate labels
    const std::string trueLabel = context->generateLabel("True");
    const std::string endLabel = context->generateLabel("End");

    /// Generate instructions for branch on void
    emit_beqz_instruction("$a0", trueLabel, ios);
    CreateObjectFromProto(context, "bool_const0", "Bool_init", ios);
    emit_jump_label_instruction(endLabel, ios);

    emit_label(trueLabel, ios);
    CreateObjectFromProto(context, "bool_const1", "Bool_init", ios);

    emit_label(endLabel, ios);
  };

  /// Function to generate code for integer complement
  auto funcComp = [context](std::iostream *ios) {
    /// Store complement of int value on the stack
    emit_lw_instruction("$a0", "$a0", OBJECT_CONTENT_OFFSET, ios);
    emit_three_registers_instruction("sub", "$a0", "$zero", "$a0", ios);
    PushAccumulatorToStack(context, ios);

    /// Create a new integer value and update its value
    CreateObjectFromProto(context, "Int_protObj", "Int_init", ios);
    emit_lw_instruction("$t0", "$sp", WORD_SIZE, ios);
    emit_sw_instruction("$t0", "$a0", OBJECT_CONTENT_OFFSET, ios);

    /// Restore stack size
    PopStack(context, 1, ios);
  };

  /// Generate code for the unary operations
  if (node->opID() == UnaryOpID::IsVoid) {
    funcIsVoidAndNot(ios);
  } else if (node->opID() == UnaryOpID::Not) {
    emit_lw_instruction("$a0", "$a0", OBJECT_CONTENT_OFFSET, ios);
    funcIsVoidAndNot(ios);
  } else {
    funcComp(ios);
  }

  return Status::Ok();
}

/// DONE
Status CodegenPass::codegen(CodegenContext *context, WhileExprNode *node,
                            std::iostream *ios) {
  /// Create labels
  const std::string loopBeginLabel = context->generateLabel("Begin");
  const std::string loopEndLabel = context->generateLabel("End");

  /// Emit label for start of loop
  emit_label(loopBeginLabel, ios);

  /// Evaluate loop condition and branch if needed
  node->loopCond()->generateCode(context, this, ios);
  emit_lw_instruction("$t0", "$a0", BOOL_CONTENT_OFFSET, ios);
  emit_beqz_instruction("$t0", loopEndLabel, ios);

  /// Generate code for loop body and jump to start of loop
  node->loopBody()->generateCode(context, this, ios);
  emit_jump_label_instruction(loopBeginLabel, ios);

  /// Emit label for end of loop construct
  emit_label(loopEndLabel, ios);

  /// Create a void return value and return
  emit_move_instruction("$a0", "$0", ios);
  return Status::Ok();
}

} // namespace cool
