#ifndef COOL_CODEGEN_CODEGEN_HELPERS_H
#define COOL_CODEGEN_CODEGEN_HELPERS_H

#include <iostream>
#include <string>

namespace cool {

static constexpr int32_t BOOL_CONTENT_OFFSET = 8;
static constexpr int32_t CLASS_ID_OFFSET = 4;
static constexpr int32_t OBJECT_CONTENT_OFFSET = 8;
static constexpr int32_t OBJECT_SIZE_OFFSET = 4;
static constexpr int32_t STRING_LENGTH_OFFSET = 8;
static constexpr int32_t STRING_CONTENT_OFFSET = 12;
static constexpr int32_t WORD_SIZE = 4;

/// Emit a MIPS instruction to add a literal value to a register and store the
/// result in a destination register, ignoring integer overflow
///
/// \param[in] dstReg destination register
/// \param[in] srcReg source register
/// \param[in] value immediate value
/// \param[out] ios output stream
void emit_addiu_instruction(const std::string &dstReg,
                            const std::string &srcReg, const int32_t value,
                            std::iostream *ios);

/// Emit a MIPS instruction to branch when register contains a value equal
/// to zero
///
/// \param[in] reg register to compare
/// \param[in] label jump label
/// \param[out] ios output stream
void emit_beqz_instruction(const std::string &reg, const std::string &label,
                           std::iostream *ios);

/// Emit a MIPS instruction to branch when register contains a value greater
/// than zero
///
/// \param[in] reg register to compare
/// \param[in] label jump label
/// \param[out] ios output stream
void emit_bgtz_instruction(const std::string &reg, const std::string &label,
                           std::iostream *ios);

/// Emit a MIPS instruction to branch when register contains a value less than
/// or equal to zero
///
/// \param[in] reg register to compare
/// \param[in] label jump label
/// \param[out] ios output stream
void emit_blez_instruction(const std::string &reg, const std::string &label,
                           std::iostream *ios);

/// Emit a MIPS instruction to branch when register contains a value less than
/// zero
///
/// \param[in] reg register to compare
/// \param[in] label jump label
/// \param[out] ios output stream
void emit_bltz_instruction(const std::string &reg, const std::string &label,
                           std::iostream *ios);

/// Emit a MIPS instruction to branch when the result of a comparison between
/// two integer values stored in a register is true
///
/// \param[in] mnemonic operation mnemonic
/// \param[in] lhsReg left hand side register
/// \param[in] rhsReg right hand side register
/// \param[in] label jump label
/// \param[out] ios output stream
void emit_compare_and_jump_instruction(const std::string &mnemonic,
                                       const std::string &lhsReg,
                                       const std::string &rhsReg,
                                       const std::string &label,
                                       std::iostream *ios);

/// Emit a MIPS jump instruction to jump to a label and store the return address
/// in $ra
///
/// \param[in] label jump label
/// \param[out] ios output stream
void emit_jump_and_link_instruction(const std::string &label,
                                    std::iostream *ios);

/// Emit a MIPS jump instruction to jump to the address stored in a register and
/// store the return address in $ra
///
/// \param[in] dstReg destination register
/// \param[out] ios output stream
void emit_jump_and_link_register_instruction(const std::string &dstReg,
                                             std::iostream *ios);

/// Emit a MIPS jump instruction to jump to a label
///
/// \param[in] label jump label
/// \param[out] ios output stream
void emit_jump_label_instruction(const std::string &label, std::iostream *ios);

/// Emit a MIPS jump instruction to jump to the address pointed by a register
///
/// \param[in] reg address register
/// \param[out] ios output stream
void emit_jump_register_instruction(const std::string &reg, std::iostream *ios);

/// Emit a MIPS label
///
/// \param[in] label label to emit
/// \param[out] ios output stream
void emit_label(const std::string &label, std::iostream *ios);

/// Emit a MIPS instruction to load an address into a register
///
/// \param[in] dstReg destination register
/// \param[in] label address to load
/// \param[out] ios output stream
void emit_la_instruction(const std::string &dstReg, const std::string &label,
                         std::iostream *ios);

/// Emit a MIPS instruction to load a byte into a register
///
/// \param[in] dstReg destination register
/// \param[in] baseReg base register
/// \param[in] offset memory offset
/// \param[out] ios output stream
void emit_lb_instruction(const std::string &dstReg, const std::string &baseReg,
                         const int32_t offset, std::iostream *ios);

/// Emit a MIPS instruction to load an integer into a register
///
/// \param[in] dstReg destination register
/// \param[in] value integer value
/// \param[out] ios output stream
void emit_li_instruction(const std::string &dstReg, const int32_t value,
                         std::iostream *ios);

/// Emit a MIPS instruction to load a word into a register from a specified
/// memory location
///
/// \param[in] dstReg destination register
/// \param[in] baseReg base register
/// \param[in] offset memory offset
/// \param[out] ios output stream
void emit_lw_instruction(const std::string &dstReg, const std::string &baseReg,
                         const int32_t offset, std::iostream *ios);

/// Emit a MIPS move instruction
///
/// \param[in] dstReg destination register
/// \param[in] srcReg source register
/// \param[out] ios output stream
void emit_move_instruction(const std::string &dstReg, const std::string &srcReg,
                           std::iostream *ios);

/// Emit a MIPS instruction to shift the bits of a register to the left by bits
/// positions and store the result into a specified register
///
/// \param[in] dstReg destination register
/// \param[in] srcReg source register
/// \param[in] bits bits to shift
/// \param[out] ios output stream
void emit_sll_instruction(const std::string &dstReg, const std::string &srcReg,
                          const size_t bits, std::iostream *ios);

/// Emit a MIPS instruction to store a word from a register into a specified
/// memory location
///
/// \param[in] srcReg source register
/// \param[in] baseReg base register
/// \param[in] offset memory offset
/// \param[out] ios output stream
void emit_sw_instruction(const std::string &srcReg, const std::string &baseReg,
                         const int32_t offset, std::iostream *ios);

/// Emit a MIPS three-register instruction
///
/// \param[in] mnemonic instruction mnemonic
/// \param[in] dstReg destination register
/// \param[in] reg1 first source register
/// \param[in] reg2 second source register
/// \param[out] ios output stream
void emit_three_registers_instruction(const std::string &mnemonic,
                                      const std::string &dstReg,
                                      const std::string &reg1,
                                      const std::string &reg2,
                                      std::iostream *ios);

} // namespace cool

#endif
