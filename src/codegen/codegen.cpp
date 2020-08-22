#include <cool/codegen/codegen.h>
#include <cool/codegen/codegen_helpers.h>
#include <cool/core/context.h>
#include <cool/ir/class.h>
#include <cool/ir/expr.h>

namespace cool {

namespace {

/// \brief Forward declaration
void CreateObjectFromProto(Context *context, const std::string &protoLabel,
                           const std::string &initLabel, std::iostream *ios);

/// \brief Helper function to compare two objects of type Int or Bool
///
/// \note The objects to compare should be stored in register $a0 and
/// on top of the stack
///
/// \param[in] context Codegen context
/// \param[out] ios output stream
void CompareBoolAndIntObjects(Context *context, std::iostream *ios) {
  /// Store lhs value in $t0
  emit_lw_instruction("$t0", "$sp", WORD_SIZE, ios);
  emit_lw_instruction("$t0", "$t0", OBJECT_CONTENT_OFFSET, ios);

  /// Store rhs value in $a0
  emit_lw_instruction("$a0", "$a0", OBJECT_CONTENT_OFFSET, ios);

  /// TODO: initialize from context
  const std::string endLabel;
  const std::string trueLabel;

  /// Compare values and generate code for false branch
  emit_compare_and_jump_instruction("beq", "$t0", "$a0", trueLabel, ios);
  CreateObjectFromProto(context, "bool_const0", "Bool_init", ios);
  emit_jump_label_instruction(endLabel, ios);

  /// Generate code for true branch
  emit_label(trueLabel, ios);
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
void CompareObjects(Context *context, std::iostream *ios) {
  /// Store lhs object address in register $t0
  emit_lw_instruction("$t0", "$sp", WORD_SIZE, ios);

  /// TODO: initialize from context
  const std::string endLabel;
  const std::string trueLabel;

  /// Compare values and generate code for false branch
  emit_compare_and_jump_instruction("beq", "$t0", "$a0", trueLabel, ios);
  CreateObjectFromProto(context, "bool_const0", "Bool_init", ios);
  emit_jump_label_instruction(endLabel, ios);

  /// Generate code for true branch
  emit_label(trueLabel, ios);
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
void CompareStringObjects(Context *context, std::iostream *ios) {
  /// Store arguments in register $a0 on the stack
  push_accumulator_to_stack(ios);

  /// Get length of first string and store it in register $a0
  emit_lw_instruction("$a0", "$sp", 2 * WORD_SIZE, ios);
  GetStringLength(context, ios);
  push_accumulator_to_stack(ios);

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

/// \brief Helper function to copy an object and initialize it
///
/// \note The object to copy should be stored in register $a0
///
/// \param[in] context Codegen context
/// \param[in] initLabel label to initialization code
/// \param[out] ios output stream
void CopyAndInitializeObject(Context *context, const std::string &initLabel,
                             std::iostream *ios) {
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

/// \brief Helper function to create a copy of an object given its unique
/// class identifyer. The pointer to the newly created object is stored in
/// register $a0
///
/// \note The class ID of the object to be initialized is expected to be stored
/// in register $a0
///
/// \param[in] context Codegen context
/// \param[out] ios output stream
void CreateObjectFromTypeID(Context *context, std::iostream *ios) {
  /// Compute offsets in prototype and init tables
  emit_move_instruction("$t1", "$a0", ios);
  emit_sll_instruction("$t1", "$t1", 2, ios);

  /// Load address of prototype object into $a0
  emit_la_instruction("$t0", "_Object_proto_table", ios);
  emit_three_registers_instruction("addu", "$t0", "$t0", "$t1", ios);
  emit_lw_instruction("$a0", "$t0", 0, ios);

  /// Create a copy of the prototype object
  emit_jump_and_link_instruction("Object.copy", ios);

  /// Load address of init function into $t0 and initialize object
  emit_la_instruction("$t0", "Init_table", ios);
  emit_three_registers_instruction("addu", "$t0", "$t0", "$t1", ios);
  emit_jump_and_link_register_instruction("$t0", ios);
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
Status GenerateDispatchCode(Context *context, CodegenPass *pass, NodeT *node,
                            FuncT fetchTableAddress, std::iostream *ios) {
  /// Evaluate parameters
  for (auto param : node->params()) {
    param->generateCode(context, pass);
    push_accumulator_to_stack(ios);
  }

  /// Evaluate expresssion if applicable, otherwise fetch self object
  if (node->expr() != nullptr) {
    node->expr()->generateCode(context, pass);
  } else {
    emit_lw_instruction("$a0", "$fp", 0, ios);
  }

  /// Fetch method table address
  fetchTableAddress();

  /// Fetch method address
  const size_t methodPosition = 0; /// TODO: fetch method position from table
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
void GetStringLength(Context *context, std::iostream *ios) {
  emit_lw_instruction("$t0", "$a0", STRING_LENGTH_OFFSET, ios);
  emit_lw_instruction("$a0", "$t0", OBJECT_CONTENT_OFFSET, ios);
}

/// \brief Helper function that returns a pointer to a default object for the
/// specified class in $a0. For classes other than built-in classes, $a0 will
/// contain a void pointer
///
/// \param[in] context Codegen context
/// \param[in] typeName class of default object
/// \param[out] ios output stream
void GenerateCodeForDefaultObject(Context *context, const std::string &typeName,
                                  std::iostream *ios) {
  /// TODO: implement functionality
}

/// \brief Helper function that determine the case statement to take and store
/// its address in register $a0
//
/// \param[in] context Codegen context
/// \param[in] node Case expression node
/// \param[out] ios output stream
void SelectCaseStatement(Context *context, CaseExprNode *node,
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
    /// Generate end of loop labels. TODO: generate from context
    const std::string endLoop;
    const std::string updateResult;

    /// Store current class identifier in register $t1
    emit_lw_instruction("$t1", "$t0", OBJECT_CLASS_OFFSET, ios);

    /// Store case class identifier in register $t2
    const size_t classID = registry->typeID(caseBinding->id());
    emit_li_instruction("$t2", classID, ios);

    /// Initialize local counter
    emit_li_instruction("$t3", 0, ios);

    /// Loop until class
    const std::string startLoop;
    emit_label(startLoop, ios);

    /// Go to next case statement if -1 has been reached
    emit_bltz_instruction("$t1", endLoop, ios);

    /// If current class is valid, check whether it is the closest found so far
    emit_compare_and_jump_instruction("beq", "$t1", "$t2", updateResult, ios);

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
    emit_label(updateResult, ios);
    emit_compare_and_jump_instruction("bgt", "$t3", "$t4", endLoop, ios);

    /// Ancestor found is the closest, update result and global counter
    emit_move_instruction("$t4", "$t3", ios);
    emit_la_instruction("$a0", caseBinding->bindingLabel(), ios);

    /// End of loop label
    emit_label(endLoop, ios);
  }
}

/// \brief Helper function that checks whether $a0 points to a void object and,
/// if so, interrupt execution
///
/// \param[in] context Codegen context
/// \param[in] errorFunc Functor / lambda to generate error handling code
/// \param[out] ios output stream
template <typename FuncT>
void TerminateExecutionIfVoid(Context *context, FuncT errorFunc,
                              std::iostream *ios) {
  /// Check whether object is void or not
  const std::string notVoidLabel;
  emit_bgtz_instruction("$a0", notVoidLabel, ios);

  /// Object is void. Generate code to handle error
  errorFunc();

  /// Emit label for non-void instruction
  emit_label(notVoidLabel, ios);
}

} // namespace

Status CodegenPass::codegen(Context *context, AssignmentExprNode *node,
                            std::iostream *ios) {
  /// Generate code for right hand side expression
  node->rhsExpr()->generateCode(context, this, ios);

  /// Update object
  auto symbolTable = context->symbolTable();
  const bool isAttribute = false;    /// TODO: read from symbol table
  const size_t variablePosition = 0; /// TODO: read from symbol table
  if (isAttribute) {
    emit_lw_instruction("$t0", "$fp", 0, ios);
    emit_sw_instruction("$a0", "$t0", variablePosition * WORD_SIZE, ios);
  } else {
    emit_sw_instruction("$a0", "$fp", variablePosition * WORD_SIZE, ios);
  }

  /// All good, stack did not change
  return Status::Ok();
}

/*
Status CodegenPass::codegen(Context *context, AttributeNode *node,
                            std::iostream *ios) {
  /// Store self object on stack
  push_accumulator_to_stack(ios);

  /// Generate attribute initialization code
  node->initExpr()->generateCode(context, this);
  emit_move_instruction("$t0", "$a0", nullptr);

  /// Update attribute
  const size_t attributePosition = 0; /// TODO: read from context
  emit_lw_instruction("$a0", "$sp", WORD_SIZE, nullptr);
  emit_sw_instruction("$t0", "$a0", attributePosition * WORD_SIZE, nullptr);

  /// Restore stack and return
  emit_addiu_instruction("$a0", "$a0", WORD_SIZE, nullptr);
  return Status::Ok();
}
*/

Status CodegenPass::codegen(Context *context,
                            BinaryExprNode<ArithmeticOpID> *node,
                            std::iostream *ios) {
  /// Evaluate left and right hand side expressions
  node->lhsExpr()->generateCode(context, this, ios);
  push_accumulator_to_stack(ios);
  node->rhsExpr()->generateCode(context, this, ios);

  /// Store lhs value on register $t0
  emit_lw_instruction("$t0", "$sp", WORD_SIZE, ios);
  emit_lw_instruction("$t0", "$t0", OBJECT_CONTENT_OFFSET, ios);

  /// Store rhs value in register $a0
  emit_lw_instruction("$a0", "$a0", OBJECT_CONTENT_OFFSET, ios);

  /// Sum values and store result in register $a0
  const std::string mnemonic = GetMnemonicFromOpType(node->opID());
  emit_three_registers_instruction(mnemonic, "$a0", "$a0", "$t0", ios);
  push_accumulator_to_stack(ios);

  /// Create new integer object from proto and return pointer in $a0
  CreateObjectFromProto(context, "Int_protObj", "Int_init", ios);

  /// Save computed value in new integer object
  emit_lw_instruction("$t0", "$sp", WORD_SIZE, ios);
  emit_sw_instruction("$t0", "$a0", OBJECT_CONTENT_OFFSET, ios);

  /// Restore stack and return
  emit_addiu_instruction("$sp", "$sp", 2 * WORD_SIZE, ios);
  return Status::Ok();
}

Status CodegenPass::codegen(Context *context,
                            BinaryExprNode<ComparisonOpID> *node,
                            std::iostream *ios) {
  /// Evaluate lhs and rhs expressions
  node->lhsExpr()->generateCode(context, this, ios);
  push_accumulator_to_stack(nullptr);
  node->rhsExpr()->generateCode(context, this, ios);

  auto funcLessThanOrEq = [context, node, ios]() {
    /// Store lhs value in $t0
    emit_lw_instruction("$t0", "$sp", WORD_SIZE, ios);
    emit_lw_instruction("$t0", "$t0", OBJECT_CONTENT_OFFSET, ios);

    /// Store rhs value in $t1
    emit_lw_instruction("$t1", "$a0", OBJECT_CONTENT_OFFSET, ios);

    const std::string endLabel;
    const std::string trueLabel;

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
  emit_addiu_instruction("$sp", "$sp", WORD_SIZE, ios);
  return Status::Ok();
}

Status CodegenPass::codegen(Context *context, BlockExprNode *node,
                            std::iostream *ios) {
  for (auto expr : node->exprs()) {
    expr->generateCode(context, this, ios);
  }
  return Status::Ok();
}

Status CodegenPass::codegen(Context *context, BooleanExprNode *node,
                            std::iostream *ios) {
  const std::string protoLabel = node->value() ? "bool_const1" : "bool_const0";
  CreateObjectFromProto(context, protoLabel, "Bool_init", ios);
  return Status::Ok();
}

Status CodegenPass::codegen(Context *context, CaseBindingNode *node) {
  /// Emit label
  emit_label(node->bindingLabel(), nullptr);

  /// Update object
  auto symbolTable = context->symbolTable();
  if (/*symbolTable->get(node->id()).isAttribute  /// TODO: implement*/
      true) {
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

Status CodegenPass::codegen(Context *context, CaseExprNode *node,
                            std::iostream *ios) {
  /// Evaluate case expression
  node->expr()->generateCode(context, this, ios);
  push_accumulator_to_stack(ios);

  /// Interrupt execution if case expression is void
  auto voidExprError = [context, node, ios]() {
    const size_t fileNameLength = 0; /// TODO: compute from context
    CreateStringObject(context, "_String_filename", fileNameLength, ios);
    emit_li_instruction("$t1", node->lineLoc(), ios);
    emit_jump_label_instruction("_case_abort2", ios);
  };
  TerminateExecutionIfVoid(context, voidExprError, ios);

  /// Select case statement
  SelectCaseStatement(context, node, ios);
  push_accumulator_to_stack(ios);

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
  const std::string endLabel;
  for (auto binding : node->cases()) {
    binding->generateCode(context, this, ios);
    emit_jump_label_instruction(endLabel, ios);
  }

  /// Emit end label, restore stack and return
  emit_label(endLabel, ios);
  emit_addiu_instruction("$sp", "$sp", 2 * WORD_SIZE, ios);
  return Status::Ok();
}

/*
Status CodegenPass::codegen(Context *context, ClassNode *node,
                            std::iostream *ios) {
  /// Initialize symbol table
  context->setCurrentClassName(node->className());
  context->initializeTables();

  /// Emit class initialization label
  emit_label(node->className() + "_init", ios);

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
  push_accumulator_to_stack(ios);
  for (auto attribute : node->attributes()) {
    const std::string typeName = attribute->typeName();
    if (typeName == "String" || typeName == "Int" || typeName == "Bool") {
      /// Generate default object for attribute
      generateDefaultObject(typeName);

      /// Update attribute
      const size_t attributePosition = 0; /// TODO: read from context
      emit_lw_instruction("$t0", "$sp", WORD_SIZE, ios);
      emit_sw_instruction("$a0", "$t0", attributePosition * WORD_SIZE, ios);
    }
  }

  /// Store self object in $a0
  emit_lw_instruction("$a0", "$sp", WORD_SIZE, ios);
  emit_addiu_instruction("$sp", "$sp", WORD_SIZE, ios);

  /// Generate code for attributes initialization
  for (auto attribute : node->attributes()) {
    attribute->generateCode(context, this, ios);
  }

  /// Generate code for methods and return
  for (auto method : node->methods()) {
    method->generateCode(context, this);
  }
  return Status::Ok();
}
*/

Status CodegenPass::codegen(Context *context, DispatchExprNode *node,
                            std::iostream *ios) {
  /// Function to fetch method table address
  auto fetchMethodAddress = []() {
    emit_la_instruction("$t0", "_Class_method_table", nullptr);
    emit_lw_instruction("$t1", "$a0", OBJECT_CLASS_OFFSET, nullptr);
    emit_three_registers_instruction("addu", "$t0", "$t0", "$t1", nullptr);
  };

  return GenerateDispatchCode(context, this, node, fetchMethodAddress, ios);
}

Status CodegenPass::codegen(Context *context, IfExprNode *node,
                            std::iostream *ios) {
  /// Generate labels
  const std::string falseLabel;
  const std::string endLabel;

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

Status CodegenPass::codegen(Context *context, LetBindingNode *node,
                            std::iostream *ios) {
  /// Generate code for right hand side expression
  if (node->hasExpr()) {
    node->expr()->generateCode(context, this, ios);
  } else {
    /// TODO: specialize for cases with SELF_TYPE / not SELF_TYPE
    GenerateCodeForDefaultObject(context, node->typeName(), ios);
  }

  return Status::Ok();
}

Status CodegenPass::codegen(Context *context, LiteralExprNode<int32_t> *node,
                            std::iostream *ios) {
  /// Create default Int object and store new value into $t0
  CreateIntObject(context, node->value(), ios);
  return Status::Ok();
}

Status CodegenPass::codegen(Context *context,
                            LiteralExprNode<std::string> *node,
                            std::iostream *ios) {
  const std::string stringProto;
  CreateStringObject(context, stringProto, node->value().length(), ios);
  return Status::Ok();
}

Status CodegenPass::codegen(Context *context, LetExprNode *node,
                            std::iostream *ios) {
  /// Fetch symbol table
  auto symbolTable = context->symbolTable();

  /// Generate code for let bindings
  for (auto binding : node->bindings()) {

    /// Enter new scope
    symbolTable->enterScope();

    /// Generate code for binding
    binding->generateCode(context, this, ios);

    /// Update symbol table (TODO) and push accumulator to stack
    push_accumulator_to_stack(ios);
  }

  /// Generate code for main let expression
  node->expr()->generateCode(context, this, ios);

  /// Unwind environment
  const size_t nCount = node->bindings().size();
  for (size_t iCount = 0; iCount < nCount; ++iCount) {
    symbolTable->exitScope();
  }

  /// Restore stack and return
  emit_addiu_instruction("$sp", "$sp", nCount * WORD_SIZE, ios);
  return Status::Ok();
}

Status CodegenPass::codegen(Context *context, MethodNode *node,
                            std::iostream *ios) {
  /// Store number of arguments in local variable
  const size_t nArgs = node->arguments().size();

  /// Nothing to do for built-in methods
  if (!node->body()) {
    return Status::Ok();
  }

  /// Fetch symbol table and enter a new scope
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
  emit_addiu_instruction("$sp", "$sp", -3 * WORD_SIZE, ios);

  /// TODO: Update environment
  for (size_t iArg = 0; iArg < nArgs; ++iArg) {
    //    symbolTable->addElement(node->arguments()[iArg]->id(), {.isAttribute
    //    = false, .position=-(iArg + 1)});
  }
  //  symbolTable->addElement("self", {.isAttribute = false, .position = 0});

  /// Generate code for method body
  node->body()->generateCode(context, this, ios);

  /// Restore return address and frame pointer
  emit_lw_instruction("$ra", "$fp", 1 * WORD_SIZE, ios);
  emit_lw_instruction("$fp", "$fp", 2 * WORD_SIZE, ios);

  /// Restore stack and jump to return address
  emit_addiu_instruction("$sp", "$sp", (3 + nArgs) * WORD_SIZE, ios);
  emit_jump_register_instruction("$ra", ios);

  /// Destroy symbol table scope and return
  symbolTable->exitScope();
  return Status::Ok();
}

Status CodegenPass::codegen(Context *context, NewExprNode *node,
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

  /// All good, return ok
  return Status::Ok();
}

Status CodegenPass::codegen(Context *context, StaticDispatchExprNode *node,
                            std::iostream *ios) {
  /// Function to fetch method table address
  auto fetchTableAddress = [context, node, ios]() {
    auto classRegistry = context->classRegistry();
    const size_t classID = classRegistry->typeID(node->callerClass());
    emit_la_instruction("$t0", "_Class_method_table", ios);
    emit_addiu_instruction("$t0", "$t0", classID * WORD_SIZE, ios);
  };

  return GenerateDispatchCode(context, this, node, fetchTableAddress, ios);
}

Status CodegenPass::codegen(Context *context, UnaryExprNode *node,
                            std::iostream *ios) {
  /// Generate code for the unary expression
  node->expr()->generateCode(context, this, ios);

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

  /// Generate code for the unary operations
  if (node->opID() == UnaryOpID::IsVoid) {
    funcIsVoidNot(ios);
  } else if (node->opID() == UnaryOpID::Not) {
    emit_lw_instruction("$a0", "$a0", OBJECT_CONTENT_OFFSET, ios);
    funcIsVoidNot(ios);
  } else {
    funcComp(ios);
  }

  /// All good, return ok
  return Status::Ok();
}

Status CodegenPass::codegen(Context *context, WhileExprNode *node,
                            std::iostream *ios) {
  /// Generate labels
  const std::string loopStartLabel;
  const std::string loopEndLabel;

  /// Emit label for start of loop
  emit_label(loopStartLabel, ios);

  /// Evaluate loop condition and branch if needed
  node->loopCond()->generateCode(context, this, ios);
  emit_lw_instruction("$t0", "$a0", BOOL_CONTENT_OFFSET, ios);
  emit_beqz_instruction("$t0", loopEndLabel, ios);

  /// Generate code for loop body and jump to start of loop
  node->loopBody()->generateCode(context, this, ios);
  emit_jump_label_instruction(loopStartLabel, ios);

  /// Emit label for end of loop construct
  emit_label(loopEndLabel, ios);

  /// Create a void return value and return
  emit_move_instruction("$a0", "$0", ios);
  return Status::Ok();
}

} // namespace cool
