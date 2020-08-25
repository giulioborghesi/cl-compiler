#include <cool/codegen/codegen_helpers.h>

#include <iomanip>
#include <ios>
#include <sstream>

namespace cool {

namespace {

static constexpr size_t INST_WIDTH = 6;
static constexpr size_t REGS_WIDTH = 6;

static const std::string INDENT = "    ";

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

} // namespace

void emit_addiu_instruction(const std::string &dstReg,
                            const std::string &srcReg, const int32_t value,
                            std::ostream *ios) {
  (*ios) << INDENT << std::left << std::setw(INST_WIDTH) << "addiu"
         << std::setw(REGS_WIDTH) << dstReg << std::setw(REGS_WIDTH) << srcReg
         << value << std::endl;
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
