#include <cool/analysis/type_check.h>
#include <cool/core/class_registry.h>
#include <cool/core/context.h>
#include <cool/ir/expr.h>

#include <memory>

#include <gtest/gtest.h>

using namespace cool;

namespace {

std::unique_ptr<Context> MakeContext() {
  return std::make_unique<Context>(new ClassRegistry());
}

} // namespace

TEST(TypeCheckTests, AddExpressionTypeCheck) {
  // Create binary expression
  auto *lhs = LiteralExprNode<int32_t>::MakeLiteralExprNode(0, 0, 0);
  auto *rhs = LiteralExprNode<int32_t>::MakeLiteralExprNode(0, 0, 0);
  std::unique_ptr<BinaryExprNode<ArithmeticOpID>> node(
      BinaryExprNode<ArithmeticOpID>::MakeBinaryExprNode(
          lhs, rhs, ArithmeticOpID::Plus, 0, 0));

  auto context = MakeContext();
  std::unique_ptr<TypeCheckPass> pass(new TypeCheckPass{});
  ASSERT_TRUE(node->visitNode(context.get(), pass.get()).isOk());
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
