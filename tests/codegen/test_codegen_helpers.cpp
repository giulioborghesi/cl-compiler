#include <cool/codegen/codegen_helpers.h>

#include <sstream>

#include <gtest/gtest.h>

namespace cool {

TEST(CodegenHelpers, BasicTests) {

  /// Test addiu emitter
  {
    std::stringstream ss;
    emit_addiu_instruction("$ra", "$t0", 10, &ss);
    ASSERT_EQ(ss.str(), "addiu $ra   $t0   10\n");
  }

  /// Test la emitter
  {
    std::stringstream ss;
    emit_la_instruction("$t0", "Int_init", &ss);
    ASSERT_EQ(ss.str(), "la    $t0   Int_init\n");
  }

  /// Test lw emitter
  {
    std::stringstream ss;
    emit_lw_instruction("$t0", "$t1", -18, &ss);
    ASSERT_EQ(ss.str(), "lw    $t0   -18($t1)\n");
  }

  /// Test sw emitter
  {
    std::stringstream ss;
    emit_sw_instruction("$t1", "$r1", 1023, &ss);
    ASSERT_EQ(ss.str(), "sw    $t1   1023($r1)\n");
  }

  /// Test helper to push accumulator
  {
    std::stringstream ss;
    push_accumulator_to_stack(&ss);
    ASSERT_EQ(ss.str(), "sw    $ra   0($sp)\naddiu $sp   $sp   -4\n");
  }
}

} // namespace cool

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
