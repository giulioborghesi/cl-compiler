#include <cool/analysis/inheritance_check.h>
#include <cool/ir/class.h>

#include <memory>

#include <gtest/gtest.h>

using namespace cool;

TEST(ClassInheritanceTests, CorrectImplementationTest) {
  std::unique_ptr<ClassNode> node{
      ClassNode::MakeClassNode("A", "B", nullptr, nullptr, 0, 0)};

  std::unique_ptr<InheritanceCheckPass> pass(new InheritanceCheckPass{});
  ASSERT_FALSE(node->visitNode(nullptr, pass.get()));
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
