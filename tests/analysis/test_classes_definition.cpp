#include <cool/analysis/classes_definition.h>
#include <cool/core/context.h>
#include <cool/core/logger_collection.h>
#include <cool/ir/class.h>

#include <utils/test_utils.h>

#include <memory>
#include <string>

#include <gtest/gtest.h>

using namespace cool;

namespace {

const std::string LOGGER_NAME = "StringLogger";

/// Helper function to initialize a context for semantic analysis
std::unique_ptr<Context> MakeContext() {
  std::shared_ptr<LoggerCollection> logger =
      std::make_shared<LoggerCollection>();
  logger->registerLogger(LOGGER_NAME, std::make_shared<StringLogger>());

  return std::make_unique<Context>(new ClassRegistry(), logger);
}

/// Helper function to extract the logger from the semantic analysis context
StringLogger *GetLogger(Context *context) {
  auto *logger = context->logger()->logger(LOGGER_NAME);
  return dynamic_cast<StringLogger *>(logger);
}

/// Helper function to create a shared pointer to a class with no attributes
///
/// \param[in] className class name
/// \param[in] parentName parent class name
/// \return a shared pointer to a class with no attributes
ClassNodePtr MakeEmptyClass(const std::string &className,
                            const std::string &parentName) {
  std::vector<GenericAttributeNodePtr> attributes;
  return ClassNode::MakeClassNode(className, parentName, attributes, false, 0,
                                  0);
}

} // namespace

TEST(ClassesDefinitionPass, ValidProgram) {
  /// Create classes
  std::vector<ClassNodePtr> classes;
  classes.push_back(MakeEmptyClass("Main", ""));
  classes.push_back(MakeEmptyClass("A", ""));
  classes.push_back(MakeEmptyClass("B", "A"));
  classes.push_back(MakeEmptyClass("C", ""));
  classes.push_back(MakeEmptyClass("D", "B"));

  /// Create program
  auto program = ProgramNode::MakeProgramNode(classes);

  /// Visit program
  auto context = MakeContext();
  std::unique_ptr<ClassesDefinitionPass> pass(new ClassesDefinitionPass{});
  auto status = program->visitNode(context.get(), pass.get());
  ASSERT_TRUE(status.isOk());
}

TEST(ClassesDefinitionPass, ClassRedefinedBuiltInClass) {
  /// Create class that inherits from
  std::vector<ClassNodePtr> classes;
  classes.push_back(MakeEmptyClass("Object", ""));

  /// Create program
  auto program = ProgramNode::MakeProgramNode(classes);

  /// Pass fails because Object cannot be redefined
  auto context = MakeContext();
  std::unique_ptr<ClassesDefinitionPass> pass(new ClassesDefinitionPass{});
  auto status = program->visitNode(context.get(), pass.get());
  ASSERT_FALSE(status.isOk());

  /// Check error message. Only one error logged
  auto *logger = GetLogger(context.get());
  ASSERT_EQ(logger->loggedMessageCount(), 1);
  ASSERT_EQ(logger->loggedMessage(0).message(),
            "Error: line 0, column 0. Class Object is a built-in class and "
            "cannot be redefined");
}

TEST(ClassesDefinitionPass, SameClassMultipleDefinitions) {
  /// Create classes
  std::vector<ClassNodePtr> classes;
  classes.push_back(MakeEmptyClass("A", ""));
  classes.push_back(MakeEmptyClass("B", ""));
  classes.push_back(MakeEmptyClass("A", ""));

  /// Create program
  auto program = ProgramNode::MakeProgramNode(classes);

  /// Pass fails because class A has been redefined
  auto context = MakeContext();
  std::unique_ptr<ClassesDefinitionPass> pass(new ClassesDefinitionPass{});
  auto status = program->visitNode(context.get(), pass.get());
  ASSERT_FALSE(status.isOk());

  /// Check error message. Only one error logged
  auto *logger = GetLogger(context.get());
  ASSERT_EQ(logger->loggedMessageCount(), 1);
  ASSERT_EQ(logger->loggedMessage(0).message(),
            "Error: line 0, column 0. Class A was defined at line 0 and cannot "
            "be redefined");
}

TEST(ClassesDefinitionPass, NonExistingParent) {
  /// Create classes
  std::vector<ClassNodePtr> classes;
  classes.push_back(MakeEmptyClass("A", "B"));

  /// Create program
  auto program = ProgramNode::MakeProgramNode(classes);

  /// Pass fails because parent class B has not been defined
  auto context = MakeContext();
  std::unique_ptr<ClassesDefinitionPass> pass(new ClassesDefinitionPass{});
  auto status = program->visitNode(context.get(), pass.get());
  ASSERT_FALSE(status.isOk());

  /// Check error message. Only one error logged
  auto *logger = GetLogger(context.get());
  ASSERT_EQ(logger->loggedMessageCount(), 1);
  ASSERT_EQ(
      logger->loggedMessage(0).message(),
      "Error: line 0, column 0. Parent class B of class A is not defined");
}

TEST(ClassesDefinitionPass, ClassInheritFromInvalidParent) {
  /// Create classes
  std::vector<ClassNodePtr> classes;
  classes.push_back(MakeEmptyClass("A", "String"));

  /// Create program
  auto program = ProgramNode::MakeProgramNode(classes);

  /// Pass fails because parent class B has not been defined
  auto context = MakeContext();
  std::unique_ptr<ClassesDefinitionPass> pass(new ClassesDefinitionPass{});
  auto status = program->visitNode(context.get(), pass.get());
  ASSERT_FALSE(status.isOk());

  /// Check error messages. Two errors logged
  auto *logger = GetLogger(context.get());
  ASSERT_EQ(logger->loggedMessageCount(), 2);
  ASSERT_EQ(
      logger->loggedMessage(0).message(),
      "Error: line 0, column 0. Parent class String of class A is not defined");
  ASSERT_EQ(logger->loggedMessage(1).message(),
            "Error: line 0, column 0. Class A cannot inherit from built-in "
            "class String");
}

TEST(ClassesDefinitionPass, CyclicDependency) {
  /// Create classes
  std::vector<ClassNodePtr> classes;
  classes.push_back(MakeEmptyClass("Main", ""));
  classes.push_back(MakeEmptyClass("A", "B"));
  classes.push_back(MakeEmptyClass("B", "A"));

  /// Create program
  auto program = ProgramNode::MakeProgramNode(classes);

  /// Pass fails because a cycle was detected in the class inheritance tree
  auto context = MakeContext();
  std::unique_ptr<ClassesDefinitionPass> pass(new ClassesDefinitionPass{});
  auto status = program->visitNode(context.get(), pass.get());
  ASSERT_FALSE(status.isOk());
  ASSERT_EQ(status.getErrorMessage(),
            "Error. Cyclic classes definition detected");

  /// Check error message. No error should be logged
  auto *logger = GetLogger(context.get());
  ASSERT_EQ(logger->loggedMessageCount(), 0);
}

TEST(ClassesDefinitionPass, SortedClasses) {
  /// Create classes
  std::vector<ClassNodePtr> classes;
  classes.push_back(MakeEmptyClass("Main", "Root"));
  classes.push_back(MakeEmptyClass("A", "C"));
  classes.push_back(MakeEmptyClass("Root", ""));
  classes.push_back(MakeEmptyClass("C", "Root"));

  // Create program
  auto program = ProgramNode::MakeProgramNode(classes);

  /// Pass succeeds
  auto context = MakeContext();
  std::unique_ptr<ClassesDefinitionPass> pass(new ClassesDefinitionPass{});
  auto status = program->visitNode(context.get(), pass.get());
  ASSERT_TRUE(status.isOk());

  /// Check sorted classes order. Expected order: Object, Main, C, A
  auto sortedClasses = program->classes();
  ASSERT_EQ(classes[2], sortedClasses[0]);
  ASSERT_EQ(classes[0], sortedClasses[1]);
  ASSERT_EQ(classes[3], sortedClasses[2]);
  ASSERT_EQ(classes[1], sortedClasses[3]);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
