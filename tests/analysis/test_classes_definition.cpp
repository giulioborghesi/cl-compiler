#include <cool/analysis/classes_definition.h>
#include <cool/core/context.h>
#include <cool/ir/class.h>

#include <memory>
#include <string>

#include <gtest/gtest.h>

using namespace cool;

namespace {

/// Helper function to initialize a context for semantic analysis
std::unique_ptr<Context> MakeContext() {
  return std::make_unique<Context>(new ClassRegistry());
}

/// Helper function to create a shared pointer to a class with no attributes
///
/// \param[in] className class name
/// \param[in] parentName parent class name
/// \return a shared pointer to a class with no attributes
ClassNodePtr MakeEmptyClass(const std::string &className,
                            const std::string &parentName) {
  std::vector<GenericAttributeNodePtr> attributes;
  return ClassNode::MakeClassNode(className, parentName, attributes, 0, 0);
}

} // namespace

TEST(ClassesDefinitionTest, ValidProgram) {
  /// Create classes
  std::vector<ClassNodePtr> classes;
  classes.push_back(MakeEmptyClass("A", ""));
  classes.push_back(MakeEmptyClass("B", "A"));
  classes.push_back(MakeEmptyClass("C", ""));
  classes.push_back(MakeEmptyClass("D", "B"));

  /// Create program
  auto program = ProgramNode::MakeProgramNode(classes);

  /// Visit program
  auto context = MakeContext();
  std::unique_ptr<ClassesDefinitionPass> pass(new ClassesDefinitionPass{});
  ASSERT_TRUE(program->visitNode(context.get(), pass.get()).isOk());
}

TEST(ClassesDefinitionTest, ClassRedefinesReservedClass) {
  /// Create class
  std::vector<ClassNodePtr> classes;
  classes.push_back(MakeEmptyClass("Object", ""));

  /// Create program
  auto program = ProgramNode::MakeProgramNode(classes);

  /// Type-check binary expression
  auto context = MakeContext();
  std::unique_ptr<ClassesDefinitionPass> pass(new ClassesDefinitionPass{});
  ASSERT_FALSE(program->visitNode(context.get(), pass.get()).isOk());
}

TEST(ClassesDefinitionTest, SameClassMultipleDefinitions) {
  /// Create classes
  std::vector<ClassNodePtr> classes;
  classes.push_back(MakeEmptyClass("A", ""));
  classes.push_back(MakeEmptyClass("B", ""));
  classes.push_back(MakeEmptyClass("A", ""));

  /// Create program
  auto program = ProgramNode::MakeProgramNode(classes);

  /// Visit program
  auto context = MakeContext();
  std::unique_ptr<ClassesDefinitionPass> pass(new ClassesDefinitionPass{});
  ASSERT_FALSE(program->visitNode(context.get(), pass.get()).isOk());
}

TEST(ClassesDefinitionTest, NonExistingParent) {
  /// Create classes
  std::vector<ClassNodePtr> classes;
  classes.push_back(MakeEmptyClass("A", "B"));

  /// Create program
  auto program = ProgramNode::MakeProgramNode(classes);

  /// Visit program
  auto context = MakeContext();
  std::unique_ptr<ClassesDefinitionPass> pass(new ClassesDefinitionPass{});
  ASSERT_FALSE(program->visitNode(context.get(), pass.get()).isOk());
}

TEST(ClassesDefinitionTest, CyclicDependency) {
  /// Create classes
  std::vector<ClassNodePtr> classes;
  classes.push_back(MakeEmptyClass("A", "B"));
  classes.push_back(MakeEmptyClass("B", "A"));

  /// Create program
  auto program = ProgramNode::MakeProgramNode(classes);

  /// Visit program
  auto context = MakeContext();
  std::unique_ptr<ClassesDefinitionPass> pass(new ClassesDefinitionPass{});
  ASSERT_FALSE(program->visitNode(context.get(), pass.get()).isOk());
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
