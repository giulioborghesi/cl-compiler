#include <cool/analysis/classes_definition.h>
#include <cool/analysis/classes_implementation.h>
#include <cool/core/context.h>
#include <cool/core/logger_collection.h>
#include <cool/ir/class.h>

#include <utils/test_utils.h>

#include <memory>
#include <string>

#include <gtest/gtest.h>

using namespace cool;

namespace {

/// Logger name
const std::string LOGGER_NAME = "StringLogger";

/// Alias for methods info type
using MethodsInfoType = std::vector<
    std::pair<std::string, std::vector<std::pair<std::string, std::string>>>>;

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

/// Helper function to create a shared pointer to a class with attributes
///
/// \param[in] className class name
/// \param[in] parentName parent class name
/// \param[in] attributesInfo list of attribute name / attribute type pairs
/// \return a shared pointer to a class with attributes
ClassNodePtr MakeClassWithAttributes(
    const std::string &className, const std::string &parentName,
    const std::vector<std::pair<std::string, std::string>> &attributesInfo =
        {}) {
  /// Create a vector of attributes from a list of attributes info
  std::vector<GenericAttributeNodePtr> attributes;
  for (auto &attributeInfo : attributesInfo) {
    attributes.push_back(AttributeNode::MakeAttributeNode(
        attributeInfo.first, attributeInfo.second, nullptr, 0, 0));
  }

  /// Create and return the class
  return ClassNode::MakeClassNode(className, parentName, attributes, false, 0,
                                  0);
}

/// Helper function to create a shared pointer to a class with methods
///
/// \param[in] className class name
/// \param[in] parentName parent class name
/// \param[in] methodsInfo list of method name / method parameters
/// \param[in] methodsReturnTypes list of method return types
/// \return a shared point to a class with methods
ClassNodePtr
MakeClassWithMethods(const std::string &className,
                     const std::string &parentName,
                     const MethodsInfoType &methodsInfo,
                     const std::vector<std::string> &methodsReturnTypes) {
  /// Create a vector of methods from a list of methods info
  std::vector<GenericAttributeNodePtr> methods;
  for (size_t i = 0; i < methodsInfo.size(); ++i) {
    std::vector<FormalNodePtr> args;
    for (auto &arg : methodsInfo[i].second) {
      args.push_back(FormalNode::MakeFormalNode(arg.first, arg.second, 0, 0));
    }

    methods.push_back(MethodNode::MakeMethodNode(
        methodsInfo[i].first, methodsReturnTypes[i], args, nullptr, 0, 0));
  }

  /// Create and return the class
  return ClassNode::MakeClassNode(className, parentName, methods, false, 0, 0);
}

} // namespace

TEST(ClassesImplementationPass, ValidProgram) {
  /// Create classes
  std::vector<ClassNodePtr> classes;
  classes.push_back(MakeClassWithAttributes("Main", "", {{"a", "A"}}));
  classes.push_back(MakeClassWithAttributes("A", "", {{"c", "C"}}));
  classes.push_back(MakeClassWithAttributes("B", "A"));
  classes.push_back(MakeClassWithAttributes("C", ""));

  /// Create program
  auto program = ProgramNode::MakeProgramNode(classes);

  /// Create semantic analysis context and passes
  auto context = MakeContext();
  std::vector<std::unique_ptr<Pass>> passes;
  passes.push_back(std::make_unique<ClassesDefinitionPass>());
  passes.push_back(std::make_unique<ClassesImplementationPass>());

  /// Both passes should succeed
  for (auto &pass : passes) {
    auto status = program->visitNode(context.get(), pass.get());
    ASSERT_TRUE(status.isOk());
  }
}

TEST(ClassesImplementationPass, UndefinedAttributeType) {
  /// Create classes
  std::vector<ClassNodePtr> classes;
  classes.push_back(MakeClassWithAttributes("Main", "", {{"a", "D"}}));
  classes.push_back(MakeClassWithAttributes("A", "", {{"c", "C"}}));
  classes.push_back(MakeClassWithAttributes("B", "A"));
  classes.push_back(MakeClassWithAttributes("C", ""));

  /// Create program
  auto program = ProgramNode::MakeProgramNode(classes);

  /// Create semantic analysis context and passes
  auto context = MakeContext();
  auto definitionPass = std::make_unique<ClassesDefinitionPass>();
  auto implementationPass = std::make_unique<ClassesImplementationPass>();

  /// Classes definition pass will succeed
  {
    auto status = program->visitNode(context.get(), definitionPass.get());
    ASSERT_TRUE(status.isOk());
  }

  /// Classes implementation pass should fail
  {
    auto status = program->visitNode(context.get(), implementationPass.get());
    ASSERT_FALSE(status.isOk());

    /// Check error message
    auto *logger = GetLogger(context.get());
    ASSERT_EQ(logger->loggedMessageCount(), 1);
    ASSERT_EQ(logger->loggedMessage(0).message(),
              "Error: line 0, column 0. Attribute a has undefined type D");
  }
}

TEST(ClassesImplementationPass, AttributeDefinedInParentClass) {
  /// Create classes
  std::vector<ClassNodePtr> classes;
  classes.push_back(MakeClassWithAttributes("Main", "", {{"a", "A"}}));
  classes.push_back(MakeClassWithAttributes("A", "", {{"c", "C"}}));
  classes.push_back(MakeClassWithAttributes("B", "A", {{"c", "C"}}));
  classes.push_back(MakeClassWithAttributes("C", ""));

  /// Create program
  auto program = ProgramNode::MakeProgramNode(classes);

  /// Create semantic analysis context and passes
  auto context = MakeContext();
  auto definitionPass = std::make_unique<ClassesDefinitionPass>();
  auto implementationPass = std::make_unique<ClassesImplementationPass>();

  /// Classes definition pass will succeed
  {
    auto status = program->visitNode(context.get(), definitionPass.get());
    ASSERT_TRUE(status.isOk());
  }

  /// Classes implementation pass should fail
  {
    auto status = program->visitNode(context.get(), implementationPass.get());
    ASSERT_FALSE(status.isOk());

    /// Check error message
    auto *logger = GetLogger(context.get());
    ASSERT_EQ(logger->loggedMessageCount(), 1);
    ASSERT_EQ(logger->loggedMessage(0).message(),
              "Error: line 0, column 0. Attribute c cannot be redefined");
  }
}

TEST(ClassesImplementationPass, AttributeCannotBeSelf) {
  /// Create classes
  std::vector<ClassNodePtr> classes;
  classes.push_back(MakeClassWithAttributes("Main", "", {{"a", "A"}}));
  classes.push_back(MakeClassWithAttributes("A", "", {{"c", "C"}}));
  classes.push_back(MakeClassWithAttributes("B", "A", {{"self", "C"}}));
  classes.push_back(MakeClassWithAttributes("C", ""));

  /// Create program
  auto program = ProgramNode::MakeProgramNode(classes);

  /// Create semantic analysis context and passes
  auto context = MakeContext();
  auto definitionPass = std::make_unique<ClassesDefinitionPass>();
  auto implementationPass = std::make_unique<ClassesImplementationPass>();

  /// Classes definition pass will succeed
  {
    auto status = program->visitNode(context.get(), definitionPass.get());
    ASSERT_TRUE(status.isOk());
  }

  /// Classes implementation pass should fail
  {
    auto status = program->visitNode(context.get(), implementationPass.get());
    ASSERT_FALSE(status.isOk());

    /// Check error message
    auto *logger = GetLogger(context.get());
    ASSERT_EQ(logger->loggedMessageCount(), 1);
    ASSERT_EQ(logger->loggedMessage(0).message(),
              "Error: line 0, column 0. 'self' is not a valid attribute name");
  }
}

TEST(ClassesImplementationPass, MethodsCannotBeRedefined) {
  /// Create classes
  std::vector<ClassNodePtr> classes;
  classes.push_back(MakeClassWithMethods("Main", "", {}, {}));
  classes.push_back(
      MakeClassWithMethods("A", "",
                           {{"method0", {{"a", "A"}, {"b", "B"}}},
                            {"method1", {{"c", "C"}}},
                            {"method0", {{"a0", "A"}, {"a1", "A"}}}},
                           {"SELF_TYPE", "SELF_TYPE", "SELF_TYPE"}));
  classes.push_back(MakeClassWithMethods("B", "A", {}, {}));
  classes.push_back(MakeClassWithMethods("C", "", {}, {}));

  /// Create program
  auto program = ProgramNode::MakeProgramNode(classes);

  /// Create semantic analysis context and passes
  auto context = MakeContext();
  auto definitionPass = std::make_unique<ClassesDefinitionPass>();
  auto implementationPass = std::make_unique<ClassesImplementationPass>();

  /// Classes definition pass will succeed
  {
    auto status = program->visitNode(context.get(), definitionPass.get());
    ASSERT_TRUE(status.isOk());
  }

  /// Classes implementation pass should fail
  {
    auto status = program->visitNode(context.get(), implementationPass.get());
    ASSERT_FALSE(status.isOk());

    /// Check error message
    auto *logger = GetLogger(context.get());
    ASSERT_EQ(logger->loggedMessageCount(), 1);
    ASSERT_EQ(logger->loggedMessage(0).message(),
              "Error: line 0, column 0. Method method0 cannot be redefined");
  }
}

TEST(ClassesImplementationPass, ParametersNamesMustBeDistinct) {
  /// Create classes
  std::vector<ClassNodePtr> classes;
  classes.push_back(MakeClassWithMethods("Main", "", {}, {}));
  classes.push_back(MakeClassWithMethods(
      "A", "",
      {{"method0", {{"a", "A"}, {"a", "B"}}}, {"method1", {{"c", "C"}}}},
      {"SELF_TYPE", "SELF_TYPE"}));
  classes.push_back(MakeClassWithMethods("B", "A", {}, {}));
  classes.push_back(MakeClassWithMethods("C", "", {}, {}));

  /// Create program
  auto program = ProgramNode::MakeProgramNode(classes);

  /// Create semantic analysis context and passes
  auto context = MakeContext();
  auto definitionPass = std::make_unique<ClassesDefinitionPass>();
  auto implementationPass = std::make_unique<ClassesImplementationPass>();

  /// Classes definition pass will succeed
  {
    auto status = program->visitNode(context.get(), definitionPass.get());
    ASSERT_TRUE(status.isOk());
  }

  /// Classes implementation pass should fail
  {
    auto status = program->visitNode(context.get(), implementationPass.get());
    ASSERT_FALSE(status.isOk());

    /// Check error message
    auto *logger = GetLogger(context.get());
    ASSERT_EQ(logger->loggedMessageCount(), 1);
    ASSERT_EQ(logger->loggedMessage(0).message(),
              "Error: line 0, column 0. Parameter a in method method0 cannot "
              "be reused");
  }
}

TEST(ClassesImplementationPass, TypesInOverloadedMethodsMustMatch) {
  /// Create classes
  std::vector<ClassNodePtr> classes;
  classes.push_back(MakeClassWithMethods(
      "Main", "", {{"method0", {{"a", "A"}, {"b", "C"}}}}, {"SELF_TYPE"}));
  classes.push_back(MakeClassWithMethods(
      "A", "Main",
      {{"method0", {{"a", "A"}, {"b", "B"}}}, {"method1", {{"c", "C"}}}},
      {"SELF_TYPE", "SELF_TYPE"}));
  classes.push_back(MakeClassWithMethods("B", "A", {}, {}));
  classes.push_back(MakeClassWithMethods("C", "", {}, {}));

  /// Create program
  auto program = ProgramNode::MakeProgramNode(classes);

  /// Create semantic analysis context and passes
  auto context = MakeContext();
  auto definitionPass = std::make_unique<ClassesDefinitionPass>();
  auto implementationPass = std::make_unique<ClassesImplementationPass>();

  /// Classes definition pass will succeed
  {
    auto status = program->visitNode(context.get(), definitionPass.get());
    ASSERT_TRUE(status.isOk());
  }

  /// Classes implementation pass should fail
  {
    auto status = program->visitNode(context.get(), implementationPass.get());
    ASSERT_FALSE(status.isOk());

    /// Check error message
    auto *logger = GetLogger(context.get());
    ASSERT_EQ(logger->loggedMessageCount(), 1);
    ASSERT_EQ(logger->loggedMessage(0).message(),
              "Error: line 0, column 0. Type of argument b in method method0 "
              "differs from parent method. Expected C, actual B");
  }
}

TEST(ClassesImplementationPass, ParameterTypeCannotBeSelfType) {
  /// Create classes
  std::vector<ClassNodePtr> classes;
  classes.push_back(MakeClassWithMethods("Main", "", {}, {}));
  classes.push_back(
      MakeClassWithMethods("A", "Main",
                           {{"method0", {{"a", "SELF_TYPE"}, {"b", "B"}}},
                            {"method1", {{"c", "C"}}}},
                           {"SELF_TYPE", "SELF_TYPE"}));
  classes.push_back(MakeClassWithMethods("B", "A", {}, {}));
  classes.push_back(MakeClassWithMethods("C", "", {}, {}));

  /// Create program
  auto program = ProgramNode::MakeProgramNode(classes);

  /// Create semantic analysis context and passes
  auto context = MakeContext();
  auto definitionPass = std::make_unique<ClassesDefinitionPass>();
  auto implementationPass = std::make_unique<ClassesImplementationPass>();

  /// Classes definition pass will succeed
  {
    auto status = program->visitNode(context.get(), definitionPass.get());
    ASSERT_TRUE(status.isOk());
  }

  /// Classes implementation pass should fail
  {
    auto status = program->visitNode(context.get(), implementationPass.get());
    ASSERT_FALSE(status.isOk());

    /// Check error message
    auto *logger = GetLogger(context.get());
    ASSERT_EQ(logger->loggedMessageCount(), 1);
    ASSERT_EQ(logger->loggedMessage(0).message(),
              "Error: line 0, column 0. Type of parameter a in method method0 "
              "cannot be SELF_TYPE");
  }
}

TEST(ClassesImplementationPass, ParameterTypeCannotBeSelf) {
  /// Create classes
  std::vector<ClassNodePtr> classes;
  classes.push_back(MakeClassWithMethods("Main", "", {}, {}));
  classes.push_back(MakeClassWithMethods(
      "A", "Main",
      {{"method0", {{"self", "A"}, {"b", "B"}}}, {"method1", {{"c", "C"}}}},
      {"SELF_TYPE", "SELF_TYPE"}));
  classes.push_back(MakeClassWithMethods("B", "A", {}, {}));
  classes.push_back(MakeClassWithMethods("C", "", {}, {}));

  /// Create program
  auto program = ProgramNode::MakeProgramNode(classes);

  /// Create semantic analysis context and passes
  auto context = MakeContext();
  auto definitionPass = std::make_unique<ClassesDefinitionPass>();
  auto implementationPass = std::make_unique<ClassesImplementationPass>();

  /// Classes definition pass will succeed
  {
    auto status = program->visitNode(context.get(), definitionPass.get());
    ASSERT_TRUE(status.isOk());
  }

  /// Classes implementation pass should fail
  {
    auto status = program->visitNode(context.get(), implementationPass.get());
    ASSERT_FALSE(status.isOk());

    /// Check error message
    auto *logger = GetLogger(context.get());
    ASSERT_EQ(logger->loggedMessageCount(), 1);
    ASSERT_EQ(logger->loggedMessage(0).message(),
              "Error: line 0, column 0. 'self' in method method0 is not a "
              "valid parameter name");
  }
}

TEST(ClassesImplementationPass, ReturnTypeInOverloadedMethodsMustMatch) {
  /// Create classes
  std::vector<ClassNodePtr> classes;
  classes.push_back(MakeClassWithMethods(
      "Main", "", {{"method0", {{"a", "A"}, {"b", "C"}}}}, {"SELF_TYPE"}));
  classes.push_back(MakeClassWithMethods(
      "A", "Main",
      {{"method0", {{"a", "A"}, {"b", "C"}}}, {"method1", {{"c", "C"}}}},
      {"A", "SELF_TYPE"}));
  classes.push_back(MakeClassWithMethods("B", "A", {}, {}));
  classes.push_back(MakeClassWithMethods("C", "", {}, {}));

  /// Create program
  auto program = ProgramNode::MakeProgramNode(classes);

  /// Create semantic analysis context and passes
  auto context = MakeContext();
  auto definitionPass = std::make_unique<ClassesDefinitionPass>();
  auto implementationPass = std::make_unique<ClassesImplementationPass>();

  /// Classes definition pass will succeed
  {
    auto status = program->visitNode(context.get(), definitionPass.get());
    ASSERT_TRUE(status.isOk());
  }

  /// Classes implementation pass should fail
  {
    auto status = program->visitNode(context.get(), implementationPass.get());
    ASSERT_FALSE(status.isOk());

    /// Check error message
    auto *logger = GetLogger(context.get());
    ASSERT_EQ(logger->loggedMessageCount(), 1);
    ASSERT_EQ(logger->loggedMessage(0).message(),
              "Error: line 0, column 0. Return type of method method0 differs "
              "from parent method. Expected SELF_TYPE, actual A");
  }
}

TEST(ClassesImplementationPass, NumberOfArgumentsInOverloadedMethodsMustMatch) {
  /// Create classes
  std::vector<ClassNodePtr> classes;
  classes.push_back(MakeClassWithMethods(
      "Main", "", {{"method0", {{"a", "A"}, {"b", "C"}}}}, {"SELF_TYPE"}));
  classes.push_back(MakeClassWithMethods(
      "A", "Main", {{"method0", {{"a", "A"}}}, {"method1", {{"c", "C"}}}},
      {"SELF_TYPE", "SELF_TYPE"}));
  classes.push_back(MakeClassWithMethods("B", "A", {}, {}));
  classes.push_back(MakeClassWithMethods("C", "", {}, {}));

  /// Create program
  auto program = ProgramNode::MakeProgramNode(classes);

  /// Create semantic analysis context and passes
  auto context = MakeContext();
  auto definitionPass = std::make_unique<ClassesDefinitionPass>();
  auto implementationPass = std::make_unique<ClassesImplementationPass>();

  /// Classes definition pass will succeed
  {
    auto status = program->visitNode(context.get(), definitionPass.get());
    ASSERT_TRUE(status.isOk());
  }

  /// Classes implementation pass should fail
  {
    auto status = program->visitNode(context.get(), implementationPass.get());
    ASSERT_FALSE(status.isOk());

    /// Check error message
    auto *logger = GetLogger(context.get());
    ASSERT_EQ(logger->loggedMessageCount(), 1);
    ASSERT_EQ(logger->loggedMessage(0).message(),
              "Error: line 0, column 0. Method method0 overrides a parent "
              "class method, but the number of arguments is not the same. "
              "Expected 2 arguments, found 1");
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
