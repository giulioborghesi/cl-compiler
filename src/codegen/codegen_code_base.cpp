#include <cool/codegen/codegen_code_base.h>
#include <cool/codegen/codegen_context.h>
#include <cool/codegen/codegen_helpers.h>
#include <cool/ir/class.h>
#include <cool/ir/expr.h>

namespace cool {

namespace {

/// Mapping from arithmetic opid to mnemonic
const std::unordered_map<ArithmeticOpID, std::string>
    ARITHMETIC_OP_TO_MNEMONIC = {{ArithmeticOpID::Plus, "add"},
                                 {ArithmeticOpID::Minus, "sub"},
                                 {ArithmeticOpID::Mult, "mul"},
                                 {ArithmeticOpID::Div, "div"}};

/// \brief Forward declarations
void GetStringLength(CodegenContext *context, std::ostream *ios);
void CreateBooleanObject(const std::string &label, std::ostream *ios);
void CreateObjectForTypeID(CodegenContext *context, std::ostream *ios);

/// \brief Helper function to compare two objects of type Int or Bool
///
/// \note The objects to compare should be stored in register $a0 and
/// on top of the stack
///
/// \param[in] context Codegen context
/// \param[out] ios output stream
void CompareBoolAndIntObjects(CodegenContext *context, std::ostream *ios) {
  /// Store lhs value in $t0
  emit_lw_instruction("$t0", "$sp", WORD_SIZE, ios);
  emit_lw_instruction("$t0", "$t0", OBJECT_CONTENT_OFFSET, ios);

  /// Store rhs value in $t1
  emit_lw_instruction("$t1", "$a0", OBJECT_CONTENT_OFFSET, ios);

  /// Create labels
  const auto compEndLabel = context->generateLabel("IntCompEnd");
  const auto sameIntLabel = context->generateLabel("IntCompSameInt");

  /// Compare values and generate code for false branch
  emit_compare_and_jump_instruction("beq", "$t0", "$t1", sameIntLabel, ios);
  CreateBooleanObject(BOOL_FALSE, ios);
  emit_jump_label_instruction(compEndLabel, ios);

  /// Generate code for true branch
  emit_label(sameIntLabel, ios);
  CreateBooleanObject(BOOL_TRUE, ios);

  /// Emit label
  emit_label(compEndLabel, ios);
}

/// \brief Compare two objects of type not equal to String, Int or Bool
///
/// \note The objects to compare should be stored in register $a0 and on the
/// stack. The objects are equal iff they point to the same object
///
/// \param[in] context Codegen context
/// \param[out] ios output stream
void CompareObjects(CodegenContext *context, std::ostream *ios) {
  /// Store lhs object address in register $t0
  emit_lw_instruction("$t0", "$sp", WORD_SIZE, ios);

  /// Create labels
  const auto compEndLabel = context->generateLabel("ObjectCompEnd");
  const auto sameObjectLabel = context->generateLabel("ObjectCompSameObject");

  /// Compare values and generate code for false branch
  emit_compare_and_jump_instruction("beq", "$t0", "$a0", sameObjectLabel, ios);
  CreateBooleanObject(BOOL_FALSE, ios);
  emit_jump_label_instruction(compEndLabel, ios);

  /// Generate code for true branch
  emit_label(sameObjectLabel, ios);
  CreateBooleanObject(BOOL_TRUE, ios);

  /// Emit label
  emit_label(compEndLabel, ios);
}

/// \brief Compare two String objects
///
/// \note The objects to compare should be stored in register $a0 and
/// on top of the stack.
///
/// \param[in] context Codegen context
/// \param[out] ios output stream
/// \DONE
void CompareStringObjects(CodegenContext *context, std::ostream *ios) {
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
  const auto compEndLabel = context->generateLabel("StringCompEnd");
  const auto sameLengthLabel = context->generateLabel("StringCompSameLength");

  /// Compare string lengths
  emit_lw_instruction("$t0", "$sp", WORD_SIZE, ios);
  emit_compare_and_jump_instruction("beq", "$a0", "$t0", sameLengthLabel, ios);

  /// Generate code for strings of different length
  CreateBooleanObject(BOOL_FALSE, ios);
  emit_jump_label_instruction(compEndLabel, ios);

  /// Lengths are the same. Compare each characters in the two strings
  emit_label(sameLengthLabel, ios);

  const auto charCompLabel = context->generateLabel("StringCompCharComp");
  const auto sameStringLabel = context->generateLabel("StringCompSameString");

  /// Store string start addresses in registers $t0 and $t1
  emit_lw_instruction("$t0", "$sp", 2 * WORD_SIZE, ios);
  emit_addiu_instruction("$t0", "$t0", STRING_CONTENT_OFFSET, ios);
  emit_lw_instruction("$t1", "$sp", 3 * WORD_SIZE, ios);
  emit_addiu_instruction("$t1", "$t1", STRING_CONTENT_OFFSET, ios);

  /// Evaluate end address of second string and store it in register $t2
  emit_lw_instruction("$t2", "$sp", WORD_SIZE, ios);
  emit_three_registers_instruction("addu", "$t2", "$t1", "$t2", ios);

  /// Compare the strings character by character
  emit_label(charCompLabel, ios);
  emit_compare_and_jump_instruction("beq", "$t1", "$t2", sameStringLabel, ios);

  /// Load characters to compare in registers $t3 and $t4
  emit_lb_instruction("$t3", "$t0", 0, ios);
  emit_lb_instruction("$t4", "$t1", 0, ios);

  /// Advance raw string pointers
  emit_addiu_instruction("$t0", "$t0", 1, ios);
  emit_addiu_instruction("$t1", "$t1", 1, ios);

  /// Go to next characters if the current ones are the same
  emit_compare_and_jump_instruction("beq", "$t3", "$t4", charCompLabel, ios);

  /// Characters differ. Create a Bool False object
  CreateBooleanObject(BOOL_FALSE, ios);
  emit_jump_label_instruction(compEndLabel, ios);

  /// Strings are the same. Create a Bool True object
  emit_label(sameStringLabel, ios);
  CreateBooleanObject(BOOL_TRUE, ios);

  /// Emit label for end of check
  emit_label(compEndLabel, ios);

  /// Restore stack
  PopStack(context, 2, ios);
}

/// \brief Helper function to create a boolean object
///
/// \param[in] label label to prototype boolean object
/// \param[out] ios output stream
void CreateBooleanObject(const std::string &label, std::ostream *ios) {
  emit_la_instruction("$a0", label, ios);
}

/// \brief Helper function to create a default object of type typeName
///
/// \param[in] context Codegen context
/// \param[in] typeName object type
/// \param[out] ios output stream
/// DONE!!
void CreateDefaultObject(CodegenContext *context, const std::string &typeName,
                         std::ostream *ios) {
  /// Create new object. Self object computes methods addresses from class ID
  if (typeName == "SELF_TYPE") {
    emit_lw_instruction("$a0", "$fp", 0, ios);
    emit_lw_instruction("$a0", "$a0", CLASS_ID_OFFSET, ios);
    CreateObjectForTypeID(context, ios);
  } else {
    CreateObjectFromProto(context, typeName, ios);
  }
}

/// \brief Helper function to create a copy of an object given its unique
/// class identifyer. The pointer to the newly created object is stored in
/// register $a0
///
/// \note The function expects the class ID of the object to be initialized to
/// be stored in register $a0
///
/// \param[in] context Codegen context
/// \param[out] ios output stream
/// DONE!!
void CreateObjectForTypeID(CodegenContext *context, std::ostream *ios) {
  /// Store prototype offset in saved register $s0
  emit_sll_instruction("$s0", "$a0", 3, ios);

  /// Load address of prototype object into $a0
  emit_la_instruction("$t0", CLASS_PROTO_TABLE, ios);
  emit_three_registers_instruction("addu", "$t0", "$t0", "$s0", ios);
  emit_lw_instruction("$a0", "$t0", 0, ios);

  /// Create a copy of the prototype object
  emit_jump_and_link_instruction("Object.copy", ios);

  /// Load address of init function into $t0 and initialize object
  emit_la_instruction("$t0", CLASS_PROTO_TABLE, ios);
  emit_three_registers_instruction("addu", "$t0", "$t0", "$s0", ios);
  emit_addiu_instruction("$t0", "$t0", 1, ios);
  emit_jump_and_link_register_instruction("$t0", ios);
}

/// \brief Create a string object for a string literal
///
/// \param[in] context Codegen context
/// \param[in] stringProto string literal label
/// \param[out] ios output stream
void CreateStringObject(CodegenContext *context, const std::string &stringProto,
                        std::ostream *ios) {
  CreateObjectFromProto(context, stringProto, "String_init", ios);
}

/// \brief Helper function that generate the code needed for method dispatch.
/// Code to fetch the method table address is specific to the dispatch type and
/// is generated by a user-passed lambda function or functor
///
/// \note The function that generates the code for fetching the method address
/// should not modify the content of register $a0. The fetched address should be
/// stored in register $t0
///
/// \param[in] context Codegen context
/// \param[in] pass Codegen pass
/// \param[in] node dispatch expression node
/// \param[in] fetchMethodAddress function to fetch the method address
/// \param[out] ios output stream
/// DONE!!
template <typename NodeT, typename FuncT>
Status GenerateDispatchCode(CodegenContext *context, CodegenCodePass *pass,
                            NodeT *node, FuncT fetchMethodAddress,
                            std::ostream *ios) {
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

  /// Fetch method address
  fetchMethodAddress();

  /// Transfer control to caller, increment stack position and return
  const size_t nParams = node->params().size();
  emit_jump_and_link_register_instruction("$t0", ios);
  context->incrementStackPosition(nParams);
  return Status::Ok();
}

/// \brief Helper function to get the mnemonic corresponding to a given
/// arithmetic operator
///
/// \param[in] opID arithmetic operator ID
/// \return the mnemonic corresponding to the given arithmetic operator ID
std::string GetMnemonicFromOpType(const ArithmeticOpID opID) {
  assert(ARITHMETIC_OP_TO_MNEMONIC.count(opID));
  return ARITHMETIC_OP_TO_MNEMONIC.find(opID)->second;
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
/// DONE!!
void GetStringLength(CodegenContext *context, std::ostream *ios) {
  emit_lw_instruction("$t0", "$a0", STRING_LENGTH_OFFSET, ios);
  emit_lw_instruction("$a0", "$t0", OBJECT_CONTENT_OFFSET, ios);
}

/// \brief Helper function that determine the case statement to take and store
/// its address in register $a0
//
/// \param[in] context Codegen context
/// \param[in] node Case expression node
/// \param[out] ios output stream
void SelectCaseStatement(CodegenContext *context, CaseExprNode *node,
                         std::ostream *ios) {
  /// Fetch class registry
  auto registry = context->classRegistry();

  /// Copy object address into $t0
  emit_move_instruction("$t0", "$a0", ios);

  /// Initialize global counter to INT_MAX and accumulator to void
  emit_move_instruction("$a0", "$zero", ios);
  emit_li_instruction("$t4", INT_MAX, ios);

  /// Loop over the cases in the case expression node
  for (auto caseBinding : node->cases()) {
    /// Create labels
    const std::string endLabel = context->generateLabel("End");
    const std::string updateLabel = context->generateLabel("UpdateCase");

    /// Store current class identifier in register $t1
    emit_lw_instruction("$t1", "$t0", CLASS_ID_OFFSET, ios);

    /// Store case class identifier in register $t2
    const size_t classID = registry->typeID(caseBinding->typeName());
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
    emit_la_instruction("$t5", "Classes_ancestors", ios);
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
                              std::ostream *ios) {
  /// Check whether object is void or not
  const std::string notVoidLabel = context->generateLabel("NotVoid");
  emit_bgtz_instruction("$a0", notVoidLabel, ios);

  /// Object is void. Generate code to handle error
  errorFunc();

  /// Emit label for non-void instruction
  emit_label(notVoidLabel, ios);
}

} // namespace

/// DONE!!
Status CodegenCodePass::codegen(CodegenContext *context,
                                AssignmentExprNode *node, std::ostream *ios) {
  /// Generate code for right hand side expression
  node->rhsExpr()->generateCode(context, this, ios);

  /// Update object
  auto symbolInfo = context->symbolTable()->get(node->id());
  const int32_t position = symbolInfo.position;
  const bool isAttribute = symbolInfo.isAttribute;
  if (isAttribute) {
    const int32_t offset = OBJECT_CONTENT_OFFSET + position * WORD_SIZE;
    emit_lw_instruction("$t0", "$fp", 0, ios);
    emit_sw_instruction("$a0", "$t0", offset, ios);
  } else {
    const int32_t offset = position * WORD_SIZE;
    emit_sw_instruction("$a0", "$fp", offset, ios);
  }

  /// All good, return
  return Status::Ok();
}

/// DONE!!
Status CodegenCodePass::codegen(CodegenContext *context,
                                BinaryExprNode<ArithmeticOpID> *node,
                                std::ostream *ios) {
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
  emit_three_registers_instruction(mnemonic, "$a0", "$t0", "$a0", ios);
  PushAccumulatorToStack(context, ios);

  /// Create a new integer object and update its value
  CreateObjectFromProto(context, "Int", ios);
  emit_lw_instruction("$t0", "$sp", WORD_SIZE, ios);
  emit_sw_instruction("$t0", "$a0", OBJECT_CONTENT_OFFSET, ios);

  /// Restore stack and return
  PopStack(context, 2, ios);
  return Status::Ok();
}

/// DONE!!
Status CodegenCodePass::codegen(CodegenContext *context,
                                BinaryExprNode<ComparisonOpID> *node,
                                std::ostream *ios) {
  if (node->opID() == ComparisonOpID::Equal) {
    return binaryEqualityCodegen(context, node, ios);
  }
  return binaryInequalityCodegen(context, node, ios);
}

/// DONE!!
Status CodegenCodePass::codegen(CodegenContext *context, BlockExprNode *node,
                                std::ostream *ios) {
  for (auto expr : node->exprs()) {
    expr->generateCode(context, this, ios);
  }
  return Status::Ok();
}

/// DONE!!
Status CodegenCodePass::codegen(CodegenContext *context, BooleanExprNode *node,
                                std::ostream *ios) {
  const std::string label = node->value() ? "Bool_const1" : "Bool_const0";
  emit_la_instruction("$a0", label, ios);
  emit_jump_and_link_instruction(OBJECT_COPY_METHOD, ios);
  return Status::Ok();
}

/// DONE
Status CodegenCodePass::codegen(CodegenContext *context, CaseBindingNode *node,
                                std::ostream *ios) {
  /// Emit label
  const std::string bindingLabel =
      context->generateLabel("Binding_" + node->id());
  emit_label(bindingLabel, ios);

  /// Enter a new symbol table scope
  auto symbolTable = context->symbolTable();
  symbolTable->enterScope();

  /// Update symbol table. Value is stored one position below top of stack
  const size_t position = context->stackPosition() - 1; /// CHECK THIS!!!
  symbolTable->addElement(node->id(), IdentifierCodegenInfo(false, position));

  /// Emit code for case binding
  node->expr()->generateCode(context, this, ios);

  /// Restore the symbol table scope and return
  symbolTable->exitScope();
  return Status::Ok();
}

/// DONE
Status CodegenCodePass::codegen(CodegenContext *context, CaseExprNode *node,
                                std::ostream *ios) {
  /// Evaluate case expression
  node->expr()->generateCode(context, this, ios);
  PushAccumulatorToStack(context, ios);

  /// Interrupt execution if case expression is void
  auto voidExprError = [context, node, ios]() {
    const size_t fileNameLength = 0; /// TODO: compute from context
    // CreateStringObject(context, "_Program_filename", fileNameLength, ios);
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
    //    CreateStringObjecte(context, ios);
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

/// DONE!!
Status CodegenCodePass::codegen(CodegenContext *context, DispatchExprNode *node,
                                std::ostream *ios) {
  auto fetchMethodAddress = [context, node, ios, this]() {
    /// Fetch dispatch table address
    (*ios) << "# Fetch method address" << std::endl;
    emit_lw_instruction("$t0", "$a0", DISPATCH_TABLE_OFFSET, ios);

    /// Fetch method address
    auto registry = context->classRegistry();
    auto typeID = registry->typeID(context->currentClassName());
    if (node->hasExpr()) {
      typeID = node->expr()->type().typeID;
    }
    auto methodTable = context->methodTable(typeID);
    const size_t position = methodTable->get(node->methodName()).position;
    emit_lw_instruction("$t0", "$t0", position * WORD_SIZE, ios);
  };

  return GenerateDispatchCode(context, this, node, fetchMethodAddress, ios);
}

/// DONE!!
Status CodegenCodePass::codegen(CodegenContext *context, IdExprNode *node,
                                std::ostream *ios) {
  /// Handle self object separately
  if (node->id() == "self") {
    emit_lw_instruction("$a0", "$fp", 0, ios);
    return Status::Ok();
  }

  /// Handle identifiers other than self
  auto symbolInfo = context->symbolTable()->get(node->id());
  const int32_t position = symbolInfo.position;
  const bool isAttribute = symbolInfo.isAttribute;
  if (isAttribute) {
    const int32_t offset = OBJECT_CONTENT_OFFSET + position * WORD_SIZE;
    emit_lw_instruction("$a0", "$fp", 0, ios);
    emit_lw_instruction("$a0", "$a0", offset, ios);
  } else {
    const int32_t offset = position * WORD_SIZE;
    emit_lw_instruction("$a0", "$fp", offset, ios);
  }
  return Status::Ok();
}

/// DONE!!
Status CodegenCodePass::codegen(CodegenContext *context, IfExprNode *node,
                                std::ostream *ios) {
  /// Create labels
  const std::string falseLabel = context->generateLabel("ElseBranch");
  const std::string endLabel = context->generateLabel("EndIf");

  /// Emit code for if expression
  node->ifExpr()->generateCode(context, this, ios);

  /// Load boolean value. Branch if false
  emit_lw_instruction("$a0", "$a0", OBJECT_CONTENT_OFFSET, ios);
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

/// DONE!!
Status CodegenCodePass::codegen(CodegenContext *context,
                                LiteralExprNode<int32_t> *node,
                                std::ostream *ios) {
  const std::string label = context->generateIntLabel(node->value());
  emit_la_instruction("$a0", label, ios);
  return Status::Ok();
}

/// DONE!!
Status CodegenCodePass::codegen(CodegenContext *context,
                                LiteralExprNode<std::string> *node,
                                std::ostream *ios) {
  const std::string label = context->generateStringLabel(node->value());
  emit_la_instruction("$a0", label, ios);
  return Status::Ok();
}

/// DONE!!
Status CodegenCodePass::codegen(CodegenContext *context, LetBindingNode *node,
                                std::ostream *ios) {
  /// Fetch symbol table
  auto symbolTable = context->symbolTable();

  /// Generate code for right hand side expression first
  if (node->hasExpr()) {
    node->expr()->generateCode(context, this, ios);
  } else {
    const std::string typeName = node->typeName();
    CreateDefaultObject(context, typeName, ios);
  }

  /// Create new scope
  symbolTable->enterScope();

  /// Add binding to symbol table
  const int32_t position = context->stackPosition();
  IdentifierCodegenInfo symbolInfo{false, position};
  symbolTable->addElement(node->id(), symbolInfo);

  /// Increment stack size and return
  return Status::Ok();
}

/// DONE!!
Status CodegenCodePass::codegen(CodegenContext *context, LetExprNode *node,
                                std::ostream *ios) {
  /// Fetch symbol table
  auto symbolTable = context->symbolTable();

  /// Generate code for let bindings
  for (auto binding : node->bindings()) {
    binding->generateCode(context, this, ios);
    PushAccumulatorToStack(context, ios);
  }

  /// Generate code for main let expression
  node->expr()->generateCode(context, this, ios);

  /// Unwind scopes
  const size_t nCount = node->bindings().size();
  for (size_t iCount = 0; iCount < nCount; ++iCount) {
    symbolTable->exitScope();
  }

  /// Restore stack and return
  PopStack(context, nCount, ios);
  return Status::Ok();
}

/// DONE!!
Status CodegenCodePass::codegen(CodegenContext *context, MethodNode *node,
                                std::ostream *ios) {
  /// Nothing to do for built-in methods
  if (!node->body()) {
    return Status::Ok();
  }

  /// Store number of arguments in local variable
  const size_t nArgs = node->arguments().size();

  /// Reset stack size, fetch symbol table and enter a new scope
  context->resetStackPosition();
  auto symbolTable = context->symbolTable();
  symbolTable->enterScope();

  /// Emit method label
  emit_label(context->currentClassName() + "." + node->id(), ios);

  /// Push stack frame
  PushStackFrame(context, ios);

  /// Update environment
  for (size_t iArg = 0; iArg < nArgs; ++iArg) {
    IdentifierCodegenInfo argInfo(false, nArgs - iArg);
    symbolTable->addElement(node->arguments()[iArg]->id(), argInfo);
  }

  /// Generate code for method body
  node->body()->generateCode(context, this, ios);

  /// Restore caller's stack frame
  PopStackFrame(context, nArgs, ios);
  emit_jump_register_instruction("$ra", ios);

  /// Exit scope and return
  symbolTable->exitScope();
  return Status::Ok();
}

/// DONE!!
Status CodegenCodePass::codegen(CodegenContext *context, NewExprNode *node,
                                std::ostream *ios) {
  const std::string typeName = node->typeName();
  CreateDefaultObject(context, typeName, ios);
  return Status::Ok();
}

/// DONE!!
Status CodegenCodePass::codegen(CodegenContext *context,
                                StaticDispatchExprNode *node,
                                std::ostream *ios) {
  auto fetchMethodAddress = [context, node, ios]() {
    auto registry = context->classRegistry();
    const size_t classID = registry->typeID(node->callerClass());

    auto methodTable = context->methodTable(classID);
    const size_t position = methodTable->get(node->methodName()).position;

    emit_la_instruction("$t0", node->callerClass() + "_dispTab", ios);
    emit_lw_instruction("$t0", "$t0", position * WORD_SIZE, ios);
  };

  return GenerateDispatchCode(context, this, node, fetchMethodAddress, ios);
}

/// DONE!!
Status CodegenCodePass::codegen(CodegenContext *context, UnaryExprNode *node,
                                std::ostream *ios) {
  if (node->opID() == UnaryOpID::Complement) {
    return unaryComplementCodegen(context, node, ios);
  }
  return unaryEqualityCodegen(context, node, ios);
}

/// DONE!!
Status CodegenCodePass::codegen(CodegenContext *context, WhileExprNode *node,
                                std::ostream *ios) {
  /// Create labels
  const std::string loopBeginLabel = context->generateLabel("LoopBegin");
  const std::string loopEndLabel = context->generateLabel("LoopEnd");

  /// Emit label for start of loop
  emit_label(loopBeginLabel, ios);

  /// Evaluate loop condition and branch if needed
  node->loopCond()->generateCode(context, this, ios);
  emit_lw_instruction("$t0", "$a0", OBJECT_CONTENT_OFFSET, ios);
  emit_beqz_instruction("$t0", loopEndLabel, ios);

  /// Generate code for loop body and jump to start of loop
  node->loopBody()->generateCode(context, this, ios);
  emit_jump_label_instruction(loopBeginLabel, ios);

  /// Emit label for end of loop construct
  emit_label(loopEndLabel, ios);

  /// Create a void return value and return
  emit_move_instruction("$a0", "$zero", ios);
  return Status::Ok();
}

Status
CodegenCodePass::binaryEqualityCodegen(CodegenContext *context,
                                       BinaryExprNode<ComparisonOpID> *node,
                                       std::ostream *ios) {
  /// Evaluate lhs and rhs expressions
  node->lhsExpr()->generateCode(context, this, ios);
  PushAccumulatorToStack(context, ios);
  node->rhsExpr()->generateCode(context, this, ios);

  /// Get lhs object type name
  auto registry = context->classRegistry();
  const auto typeID = node->lhsExpr()->type().typeID;
  const std::string typeName = registry->className(typeID);

  /// Take decision based on object type
  if (typeName == "Int" || typeName == "Bool") {
    CompareBoolAndIntObjects(context, ios);
  } else if (typeName == "String") {
    CompareStringObjects(context, ios);
  } else {
    CompareObjects(context, ios);
  }

  /// Pop stack and return
  PopStack(context, 1, ios);
  return Status::Ok();
}

Status
CodegenCodePass::binaryInequalityCodegen(CodegenContext *context,
                                         BinaryExprNode<ComparisonOpID> *node,
                                         std::ostream *ios) {
  /// Evaluate left and right hand side expressions
  node->lhsExpr()->generateCode(context, this, ios);
  PushAccumulatorToStack(context, ios);
  node->rhsExpr()->generateCode(context, this, ios);

  /// Store lhs value in $t0
  emit_lw_instruction("$t0", "$sp", WORD_SIZE, ios);
  emit_lw_instruction("$t0", "$t0", OBJECT_CONTENT_OFFSET, ios);

  /// Store rhs value in $t1
  emit_lw_instruction("$t1", "$a0", OBJECT_CONTENT_OFFSET, ios);

  /// End label
  const std::string endLabel = context->generateLabel("BinaryCompEnd");
  const std::string trueLabel = context->generateLabel("BinaryCompTrueBranch");

  /// Compare values and branch as needed
  const std::string mnemonic = GetMnemonicFromOpType(node->opID());
  emit_compare_and_jump_instruction(mnemonic, "$t0", "$t1", trueLabel, ios);
  CreateBooleanObject("Bool_const0", ios);
  emit_jump_label_instruction(endLabel, ios);

  emit_label(trueLabel, ios);
  CreateBooleanObject("Bool_const1", ios);

  emit_label(endLabel, ios);
  PopStack(context, 1, ios);
  return Status::Ok();
}

Status CodegenCodePass::unaryComplementCodegen(CodegenContext *context,
                                               UnaryExprNode *node,
                                               std::ostream *ios) {
  /// Generate code for the unary expression
  node->expr()->generateCode(context, this, ios);

  /// Store complement of int value on the stack
  emit_lw_instruction("$a0", "$a0", OBJECT_CONTENT_OFFSET, ios);
  emit_neg_instruction("$a0", "$a0", ios);
  PushAccumulatorToStack(context, ios);

  /// Create a new integer object and update its value
  CreateObjectFromProto(context, "Int", ios);
  emit_lw_instruction("$t0", "$sp", WORD_SIZE, ios);
  emit_sw_instruction("$t0", "$a0", OBJECT_CONTENT_OFFSET, ios);

  /// Restore stack
  PopStack(context, 1, ios);
  return Status::Ok();
}

Status CodegenCodePass::unaryEqualityCodegen(CodegenContext *context,
                                             UnaryExprNode *node,
                                             std::ostream *ios) {
  /// Generate code for the unary expression
  node->expr()->generateCode(context, this, ios);
  if (node->opID() == UnaryOpID::Not) {
    emit_lw_instruction("$a0", "$a0", OBJECT_CONTENT_OFFSET, ios);
  }

  /// Generate labels
  const std::string trueLabel = context->generateLabel("UnaryEqTrueBranch");
  const std::string endLabel = context->generateLabel("UnaryEqEnd");

  /// Compare for equality with zero and branch as needed
  emit_beqz_instruction("$a0", trueLabel, ios);
  CreateBooleanObject(BOOL_FALSE, ios);
  emit_jump_label_instruction(endLabel, ios);

  emit_label(trueLabel, ios);
  CreateBooleanObject(BOOL_TRUE, ios);

  emit_label(endLabel, ios);
  return Status::Ok();
}

} // namespace cool
