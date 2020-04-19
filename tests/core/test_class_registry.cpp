#include <cool/core/class_registry.h>
#include <cool/ir/class.h>

#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>

namespace cool {

namespace {

/// Helper method to create a shared pointer to a class node
///
/// \param[in] className class name
/// \param[in] parentClassName parent class name
/// \return a shared pointer to a class node for the specified class
std::shared_ptr<ClassNode> CreateClassNode(const std::string &className,
                                           const std::string &parentClassName) {
  std::vector<GenericAttributeNodePtr> attributes;

  return std::shared_ptr<ClassNode>(
      ClassNode::MakeClassNode(className, parentClassName, attributes, 0, 0));
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

  /// Attempting to add a class that is already in the registry
  auto statusA2 = registry.addClass(classA2);
  ASSERT_FALSE(statusA2.isOk());
}

TEST(ClassRegistry, NonExistentParentClass) {
  auto classA = CreateClassNode("A", "");

  ClassRegistry registry{};

  auto statusAddClassA = registry.addClass(classA);
  ASSERT_TRUE(statusAddClassA.isOk());

  /// Inheritance tree is well formed
  {
    auto statusCheckInheritance = registry.checkInheritanceTree();
    ASSERT_TRUE(statusCheckInheritance.isOk());
  }

  /// Add node with non-existing parent class
  auto classB = CreateClassNode("B", "C");
  auto statusAddClassB = registry.addClass(classB);
  ASSERT_TRUE(statusAddClassB.isOk());

  /// Inheritance tree is now ill formed
  {
    auto statusCheckInheritance = registry.checkInheritanceTree();
    ASSERT_FALSE(statusCheckInheritance.isOk());
    ASSERT_EQ(statusCheckInheritance.getErrorMessage(),
              "Error: parent class is not defined");
  }
}

TEST(ClassRegistry, CyclicDependencyInClassInheritanceTree) {
  auto classA = CreateClassNode("A", "C");
  auto classB = CreateClassNode("B", "A");
  auto classC = CreateClassNode("C", "B");

  ClassRegistry registry{};

  auto statusAddClassA = registry.addClass(classA);
  ASSERT_TRUE(statusAddClassA.isOk());

  auto statusAddClassB = registry.addClass(classB);
  ASSERT_TRUE(statusAddClassB.isOk());

  auto statusAddClassC = registry.addClass(classC);
  ASSERT_TRUE(statusAddClassC.isOk());

  auto statusCheckInheritance = registry.checkInheritanceTree();
  ASSERT_FALSE(statusCheckInheritance.isOk());

  ASSERT_EQ(statusCheckInheritance.getErrorMessage(),
            "Error: cyclic class dependency detected");
}

TEST(ClassRegistry, TypeRelationships) {
  auto classA = CreateClassNode("A", "");
  auto classB = CreateClassNode("B", "A");
  auto classC = CreateClassNode("C", "B");
  auto classD = CreateClassNode("D", "A");

  ClassRegistry registry{};
  auto statusA = registry.addClass(classA);
  ASSERT_TRUE(statusA.isOk());

  auto statusB = registry.addClass(classB);
  ASSERT_TRUE(statusB.isOk());

  auto statusC = registry.addClass(classC);
  ASSERT_TRUE(statusC.isOk());

  auto statusD = registry.addClass(classD);
  ASSERT_TRUE(statusC.isOk());

  /// Define types
  ExprType typeA{.typeID = registry.typeID("A"), .isSelf = false};
  ExprType typeB{.typeID = registry.typeID("B"), .isSelf = false};
  ExprType typeC{.typeID = registry.typeID("C"), .isSelf = false};
  ExprType typeD{.typeID = registry.typeID("D"), .isSelf = false};
  ExprType typeO{.typeID = registry.typeID("Object"), .isSelf = false};

  /// C is a descendant of A
  ASSERT_TRUE(registry.conformTo(typeC, typeA));

  /// A is a descendant of Object
  ASSERT_TRUE(registry.conformTo(typeA, typeO));

  /// A is not a descendant of B
  ASSERT_FALSE(registry.conformTo(typeA, typeB));

  /// Least common ancestor of B and D is A
  ASSERT_EQ(typeA.typeID, registry.leastCommonAncestor(typeB, typeD).typeID);

  /// Least common ancestor of C and A is A
  ASSERT_EQ(typeA.typeID, registry.leastCommonAncestor(typeA, typeC).typeID);
}

} // namespace cool

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
