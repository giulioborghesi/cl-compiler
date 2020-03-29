#include <cool/core/class_registry.h>
#include <cool/ir/class.h>

#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>

namespace cool {

namespace {

std::shared_ptr<ClassNode> CreateClassNode(const std::string &className,
                                           const std::string &parentClassName) {
  std::vector<std::shared_ptr<AttributeNode>> attributes;
  std::vector<std::shared_ptr<MethodNode>> methods;

  return std::shared_ptr<ClassNode>(ClassNode::MakeClassNode(
      className, parentClassName, &attributes, &methods, 0, 0));
}

} // namespace

TEST(ClassRegistry, DuplicatedClassInRegistry) {
  auto classA1 = CreateClassNode("A", "");
  auto classB1 = CreateClassNode("B", "");
  auto classA2 = CreateClassNode("A", "");

  ClassRegistry registry{};

  auto statusA1 = registry.addClass(classA1);
  ASSERT_TRUE(statusA1.isOk());

  auto statusB1 = registry.addClass(classB1);
  ASSERT_TRUE(statusB1.isOk());

  auto statusA2 = registry.addClass(classA2);
  ASSERT_FALSE(statusA2.isOk());
}

TEST(ClassRegistry, NonExistentParentClass) {
  auto classA = CreateClassNode("A", "B");

  ClassRegistry registry{};

  auto statusAddClass = registry.addClass(classA);
  ASSERT_TRUE(statusAddClass.isOk());

  auto statusCheckInheritance = registry.checkInheritanceTree();
  ASSERT_FALSE(statusCheckInheritance.isOk());

  ASSERT_EQ(statusCheckInheritance.getErrorMessage(),
            "Error: parent class is not defined");
}

} // namespace cool

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
