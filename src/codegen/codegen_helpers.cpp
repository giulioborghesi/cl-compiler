#include <cool/codegen/codegen_context.h>
#include <cool/codegen/codegen_helpers.h>

#include <iomanip>
#include <ios>
#include <sstream>

namespace cool {

namespace {

/// Instruction and register fields widths
static constexpr size_t INST_WIDTH = 6;
static constexpr size_t DIRS_WIDTH = 8;
static constexpr size_t REGS_WIDTH = 6;

/// Instruction indent
static const std::string INDENT = "     ";

void emit_bg_instruction(const std::string &mnemonic, const std::string &reg,
                         const std::string &label, std::ostream *ios) {
  (*ios) << INDENT << std::left << std::setw(INST_WIDTH) << mnemonic
         << std::setw(REGS_WIDTH) << reg << label << std::endl;
}

void emit_jump_instruction(const std::string &mnemonic, const std::string &arg,
                           std::ostream *ios) {
  (*ios) << INDENT << std::left << std::setw(INST_WIDTH) << mnemonic << arg
         << std::endl;
}

template <typename T>
void emit_mips_data_line_impl(const std::string &dataType, T value,
                              std::ostream *ios) {
  (*ios) << INDENT << std::left << std::setw(DIRS_WIDTH) << dataType << value
         << std::endl;
}

} // namespace

void CopyAndInitializeObject(CodegenContext *context,
                             const std::string &initLabel, std::ostream *ios) {
  /// Create a copy of the object and store it on the stack
  emit_jump_and_link_instruction("Object.copy", ios);

  /// Initialize object
  emit_jump_and_link_instruction(initLabel, ios);
}

void CreateIntObject(CodegenContext *context, const int32_t value,
                     std::ostream *ios) {
  CreateObjectFromProto(context, "Int_protObj", "Int_init", ios);
  emit_li_instruction("$t0", value, ios);

  /// Update Int value and return
  emit_sw_instruction("$t0", "$a0", OBJECT_CONTENT_OFFSET, ios);
}

void CreateObjectFromProto(CodegenContext *context,
                           const std::string &protoLabel,
                           const std::string &initLabel, std::ostream *ios) {
  /// Load address of prototype object into $a0
  emit_la_instruction("$a0", protoLabel, ios);

  /// Copy the object and initialize it
  CopyAndInitializeObject(context, initLabel, ios);
}

void CreateStringObject(CodegenContext *context,
                        const std::string &literalProto,
                        const size_t stringLength, std::ostream *ios) {
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

void PopStackFrame(CodegenContext *context, std::ostream *ios) {
  emit_lw_instruction("$ra", "$fp", 1 * WORD_SIZE, ios);
  emit_lw_instruction("$fp", "$fp", 2 * WORD_SIZE, ios);
  PopStack(context, 3, ios);
}

void PopStack(CodegenContext *context, const size_t count, std::ostream *ios) {
  emit_addiu_instruction("$sp", "$sp", count * WORD_SIZE, ios);
  context->decrementStackSize(count);
}

void PushAccumulatorToStack(CodegenContext *context, std::ostream *ios) {
  emit_sw_instruction("$a0", "$sp", 0, ios);
  emit_addiu_instruction("$sp", "$sp", -4, ios);
  context->incrementStackSize(1);
}

void PushStack(CodegenContext *context, const size_t count, std::ostream *ios) {
  emit_addiu_instruction("$sp", "$sp", -count * WORD_SIZE, ios);
  context->incrementStackSize(count);
}

void PushStackFrame(CodegenContext *context, std::ostream *ios) {
  emit_sw_instruction("$a0", "$fp", 0 * WORD_SIZE, ios);
  emit_sw_instruction("$ra", "$fp", 1 * WORD_SIZE, ios);
  emit_sw_instruction("$fp", "$fp", 2 * WORD_SIZE, ios);
  emit_move_instruction("$fp", "$sp", ios);
  PushStack(context, 3, ios);
}

void emit_addiu_instruction(const std::string &dstReg,
                            const std::string &srcReg, const int32_t value,
                            std::ostream *ios) {
  (*ios) << INDENT << std::left << std::setw(INST_WIDTH) << "addiu"
         << std::setw(REGS_WIDTH) << dstReg << std::setw(REGS_WIDTH) << srcReg
         << value << std::endl;
}

void emit_ascii_data(const std::string &literal, std::ostream *ios) {
  emit_mips_data_line_impl(".ascii", literal, ios);
}

void emit_beqz_instruction(const std::string &reg, const std::string &label,
                           std::ostream *ios) {
  emit_bg_instruction("beqz", reg, label, ios);
}

void emit_bgtz_instruction(const std::string &reg, const std::string &label,
                           std::ostream *ios) {
  emit_bg_instruction("bgtz", reg, label, ios);
}

void emit_blez_instruction(const std::string &reg, const std::string &label,
                           std::ostream *ios) {
  emit_bg_instruction("blez", reg, label, ios);
}

void emit_bltz_instruction(const std::string &reg, const std::string &label,
                           std::ostream *ios) {
  emit_bg_instruction("bltz", reg, label, ios);
}

void emit_compare_and_jump_instruction(const std::string &mnemonic,
                                       const std::string &lhsReg,
                                       const std::string &rhsReg,
                                       const std::string &label,
                                       std::ostream *ios) {
  (*ios) << INDENT << std::left << std::setw(INST_WIDTH) << mnemonic
         << std::setw(REGS_WIDTH) << lhsReg << std::setw(REGS_WIDTH) << rhsReg
         << label << std::endl;
}

void emit_global_declaration(const std::string &label, std::ostream *ios) {
  emit_mips_data_line_impl(".globl", label, ios);
}

void emit_word_data(const int32_t value, std::ostream *ios) {
  emit_mips_data_line_impl(".word", value, ios);
}

void emit_word_data(const std::string &value, std::ostream *ios) {
  emit_mips_data_line_impl(".word", value, ios);
}

void emit_directive(const std::string &directive, std::ostream *ios) {
  (*ios) << std::endl;
  (*ios) << INDENT << directive << std::endl;
}

void emit_jump_label_instruction(const std::string &label, std::ostream *ios) {
  emit_jump_instruction("j", label, ios);
}

void emit_jump_register_instruction(const std::string &reg, std::ostream *ios) {
  emit_jump_instruction("jr", reg, ios);
}

void emit_jump_and_link_instruction(const std::string &label,
                                    std::ostream *ios) {
  emit_jump_instruction("jal", label, ios);
}

void emit_jump_and_link_register_instruction(const std::string &dstReg,
                                             std::ostream *ios) {
  emit_jump_instruction("jalr", dstReg, ios);
}

void emit_label(const std::string &label, std::ostream *ios) {
  (*ios) << std::endl;
  (*ios) << label << ":" << std::endl;
}

void emit_la_instruction(const std::string &dstReg, const std::string &label,
                         std::ostream *ios) {
  (*ios) << INDENT << std::left << std::setw(INST_WIDTH) << "la"
         << std::setw(REGS_WIDTH) << dstReg << label << std::endl;
}

void emit_lb_instruction(const std::string &dstReg, const std::string &baseReg,
                         const int32_t offset, std::ostream *ios) {
  (*ios) << INDENT << std::left << std::setw(INST_WIDTH) << "lb"
         << std::setw(REGS_WIDTH) << dstReg << offset << "(" << baseReg << ")"
         << std::endl;
}

void emit_li_instruction(const std::string &dstReg, const int32_t value,
                         std::ostream *ios) {
  (*ios) << INDENT << std::left << std::setw(INST_WIDTH) << "li"
         << std::setw(REGS_WIDTH) << dstReg << value << std::endl;
}

void emit_lw_instruction(const std::string &dstReg, const std::string &baseReg,
                         const int32_t offset, std::ostream *ios) {
  (*ios) << INDENT << std::left << std::setw(INST_WIDTH) << "lw"
         << std::setw(REGS_WIDTH) << dstReg << offset << "(" << baseReg << ")"
         << std::endl;
}

void emit_move_instruction(const std::string &dstReg, const std::string &srcReg,
                           std::ostream *ios) {
  (*ios) << INDENT << std::left << std::setw(INST_WIDTH) << "move"
         << std::setw(REGS_WIDTH) << dstReg << srcReg << std::endl;
}

void emit_object_label(const std::string &label, std::ostream *ios) {
  (*ios) << std::endl;
  (*ios) << INDENT << std::left << std::setw(INST_WIDTH) << ".word" << -1
         << std::endl;
  (*ios) << label << ":" << std::endl;
}

void emit_sll_instruction(const std::string &dstReg, const std::string &srcReg,
                          const size_t bits, std::ostream *ios) {
  (*ios) << INDENT << std::left << std::setw(INST_WIDTH) << "sll"
         << std::setw(REGS_WIDTH) << dstReg << std::setw(REGS_WIDTH) << srcReg
         << bits << std::endl;
}

void emit_sw_instruction(const std::string &srcReg, const std::string &baseReg,
                         const int32_t offset, std::ostream *ios) {
  (*ios) << INDENT << std::left << std::setw(INST_WIDTH) << "sw"
         << std::setw(REGS_WIDTH) << srcReg << offset << "(" << baseReg << ")"
         << std::endl;
}

void emit_three_registers_instruction(const std::string &mnemonic,
                                      const std::string &dstReg,
                                      const std::string &reg1,
                                      const std::string &reg2,
                                      std::ostream *ios) {
  (*ios) << INDENT << std::left << std::setw(INST_WIDTH) << mnemonic
         << std::setw(REGS_WIDTH) << dstReg << std::setw(REGS_WIDTH) << reg1
         << reg2 << std::endl;
}

} // namespace cool
