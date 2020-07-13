#include <cool/analysis/classes_implementation.h>
#include <cool/analysis/type_check.h>
#include <cool/core/class_registry.h>
#include <cool/core/context.h>
#include <cool/core/logger_collection.h>
#include <cool/ir/class.h>
#include <cool/ir/expr.h>

#include <utils/test_utils.h>

#include <memory>

#include <gtest/gtest.h>

using namespace cool;

namespace {

/// Logger name
const std::string LOGGER_NAME = "StringLogger";

/// Alias for methods info type
using MethodsInfoType = std::vector<
    std::pair<std::string, std::vector<std::pair<std::string, std::string>>>>;

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
        methodsInfo[i].first, methodsReturnTypes[i], args, 0, 0));
  }

  /// Create and return the class
  return ClassNode::MakeClassNode(className, parentName, methods, false, 0, 0);
}

/// Helper function to initialize a context for type checking
std::unique_ptr<Context> MakeContextWithDefaultClasses() {
  std::shared_ptr<LoggerCollection> logger =
      std::make_shared<LoggerCollection>();
  /// Create context
  logger->registerLogger(LOGGER_NAME, std::make_shared<StringLogger>());
  auto context = std::make_unique<Context>(new ClassRegistry(), logger);

  /// Add object class to context
  std::vector<GenericAttributeNodePtr> attributes;
  context->classRegistry()->addClass(
      ClassNode::MakeClassNode("Object", "", attributes, false, 0, 0));
  context->setCurrentClassName("Object");
  context->initializeTables();

  /// Add supplementary classes to context
  std::vector<std::pair<std::string, std::string>> classes = {
      {"A", "Object"},   {"B", "A"},         {"D", "Object"},
      {"Int", "Object"}, {"Bool", "Object"}, {"String", "Object"}};
  for (auto &[className, parentClassName] : classes) {
    context->classRegistry()->addClass(ClassNode::MakeClassNode(
        className, parentClassName, attributes, false, 0, 0));
    context->setCurrentClassName(className);
    context->initializeTables();
  }

  /// Add class with methods to context
  auto methodsClassA = MakeClassWithMethods(
      "Z", "A", {{"methodA", {{"b", "B"}}}, {"methodB", {{"d", "D"}}}},
      {"SELF_TYPE", "B"});
  context->classRegistry()->addClass(methodsClassA);
  context->setCurrentClassName("Z");
  context->initializeTables();

  // Initialize method table
  auto implementationPass = std::make_unique<ClassesImplementationPass>();
  for (auto methodNode : methodsClassA->methods()) {
    implementationPass->visit(context.get(), methodNode.get());
  }

  auto methodsClassB = MakeClassWithMethods(
      "X", "Z", {{"methodA", {{"b", "B"}}}, {"methodB", {{"d", "D"}}}},
      {"SELF_TYPE", "B"});
  context->classRegistry()->addClass(methodsClassB);
  context->setCurrentClassName("X");
  context->initializeTables();

  // Initialize method table
  for (auto methodNode : methodsClassB->methods()) {
    implementationPass->visit(context.get(), methodNode.get());
  }

  /// Set A to current class and return
  context->setCurrentClassName("A");
  return context;
} // namespace

/// Helper function to extract the logger from the semantic analysis context
StringLogger *GetLogger(Context *context) {
  auto *logger = context->logger()->logger(LOGGER_NAME);
  return dynamic_cast<StringLogger *>(logger);
}

} // namespace

/// AssignmentExprNode
TEST(TypeCheckTests, AssignmentExprNodeTests) {
  /// Create context
  auto context = MakeContextWithDefaultClasses();
  auto *registry = context->classRegistry();

  /// Create pass
  auto typeCheckPass = std::make_unique<TypeCheckPass>();

  /// Create nodes and register symbols
  auto nodeA = IdExprNode::MakeIdExprNode("a", 0, 0);
  context->symbolTable()->addElement("a", registry->toType("A"));

  auto nodeB = IdExprNode::MakeIdExprNode("b", 0, 0);
  context->symbolTable()->addElement("b", registry->toType("B"));

  auto nodeD = IdExprNode::MakeIdExprNode("d", 0, 0);
  context->symbolTable()->addElement("d", registry->toType("D"));

  auto nodeSelf = IdExprNode::MakeIdExprNode("self", 0, 0);
  context->symbolTable()->addElement("self", registry->toSelfType("A"));

  /// Successfull assignment to same-type variable
  {
    auto node = AssignmentExprNode::MakeAssignmentExprNode("a", nodeA, 0, 0);
    auto status = typeCheckPass->visit(context.get(), node.get());
    ASSERT_TRUE(status.isOk());
    ASSERT_EQ(node->type(), registry->toType("A"));
  }

  /// Successfull assignment to variable of derived type
  {
    auto node = AssignmentExprNode::MakeAssignmentExprNode("a", nodeB, 0, 0);
    auto status = typeCheckPass->visit(context.get(), node.get());
    ASSERT_TRUE(status.isOk());
    ASSERT_EQ(node->type(), registry->toType("B"));
  }

  /// Assignment to an undefined variable
  {
    auto node = AssignmentExprNode::MakeAssignmentExprNode("c", nodeA, 0, 0);
    auto status = typeCheckPass->visit(context.get(), node.get());
    ASSERT_FALSE(status.isOk());

    /// Check error message
    auto *logger = GetLogger(context.get());
    ASSERT_EQ(logger->loggedMessageCount(), 1);
    ASSERT_EQ(logger->loggedMessage(0).message(),
              "Error: line 0, column 0. Variable c is not defined");
    logger->reset();
  }

  /// Assignment to self
  {
    auto node = AssignmentExprNode::MakeAssignmentExprNode("self", nodeA, 0, 0);
    auto status = typeCheckPass->visit(context.get(), node.get());
    ASSERT_FALSE(status.isOk());

    /// Check error message
    auto *logger = GetLogger(context.get());
    ASSERT_EQ(logger->loggedMessageCount(), 1);
    ASSERT_EQ(logger->loggedMessage(0).message(),
              "Error: line 0, column 0. Cannot assign to 'self'");
    logger->reset();
  }

  /// Invalid type of right hand side expression
  {
    auto node = AssignmentExprNode::MakeAssignmentExprNode("a", nodeD, 0, 0);
    auto status = typeCheckPass->visit(context.get(), node.get());
    ASSERT_FALSE(status.isOk());

    /// Check error message
    auto *logger = GetLogger(context.get());
    ASSERT_EQ(logger->loggedMessageCount(), 1);
    ASSERT_EQ(logger->loggedMessage(0).message(),
              "Error: line 0, column 0. Type of right hand side expression "
              "evaluates to D, which is not a "
              "subtype of A");
    logger->reset();
  }
}

// BinaryExprNode (ArithmeticOpID)
TEST(TypeCheckTests, BinaryExprNodeArithmeticOpsTests) {
  /// Create context
  auto context = MakeContextWithDefaultClasses();
  auto *registry = context->classRegistry();

  /// Create pass
  auto typeCheckPass = std::make_unique<TypeCheckPass>();

  /// Create nodes and register symbols
  auto nodeA = IdExprNode::MakeIdExprNode("a", 0, 0);
  context->symbolTable()->addElement("a", registry->toType("Int"));

  auto nodeB = IdExprNode::MakeIdExprNode("b", 0, 0);
  context->symbolTable()->addElement("b", registry->toType("Int"));

  auto nodeC = IdExprNode::MakeIdExprNode("c", 0, 0);
  context->symbolTable()->addElement("c", registry->toSelfType("A"));

  /// Successfull arithmetic expression
  {
    auto node = BinaryExprNode<ArithmeticOpID>::MakeBinaryExprNode(
        nodeA, nodeB, ArithmeticOpID::Plus, 0, 0);
    auto status = typeCheckPass->visit(context.get(), node.get());
    ASSERT_TRUE(status.isOk());
    ASSERT_EQ(node->type(), registry->toType("Int"));
  }

  /// Left hand side is not an integer expression
  {
    auto node = BinaryExprNode<ArithmeticOpID>::MakeBinaryExprNode(
        nodeC, nodeB, ArithmeticOpID::Plus, 0, 0);
    auto status = typeCheckPass->visit(context.get(), node.get());
    ASSERT_FALSE(status.isOk());

    /// Check error message
    auto *logger = GetLogger(context.get());
    ASSERT_EQ(logger->loggedMessageCount(), 1);
    ASSERT_EQ(logger->loggedMessage(0).message(),
              "Error: line 0, column 0. Arithmetic expressions between "
              "non-integer types are not supported");
    logger->reset();
  }

  /// Right hand side is not an integer expression
  {
    auto node = BinaryExprNode<ArithmeticOpID>::MakeBinaryExprNode(
        nodeA, nodeC, ArithmeticOpID::Plus, 0, 0);
    auto status = typeCheckPass->visit(context.get(), node.get());
    ASSERT_FALSE(status.isOk());

    /// Check error message
    auto *logger = GetLogger(context.get());
    ASSERT_EQ(logger->loggedMessageCount(), 1);
    ASSERT_EQ(logger->loggedMessage(0).message(),
              "Error: line 0, column 0. Arithmetic expressions between "
              "non-integer types are not supported");
    logger->reset();
  }
}

// BinaryExprNode (ComparisonOpID::Equal)
TEST(TypeCheckTests, BinaryExprNodeComparisonOpsTests) {
  /// Create context
  auto context = MakeContextWithDefaultClasses();
  auto *registry = context->classRegistry();

  /// Create pass
  auto typeCheckPass = std::make_unique<TypeCheckPass>();

  /// Create nodes and register symbols
  auto nodeI = IdExprNode::MakeIdExprNode("int", 0, 0);
  context->symbolTable()->addElement("int", registry->toType("Int"));

  auto nodeA = IdExprNode::MakeIdExprNode("a", 0, 0);
  context->symbolTable()->addElement("a", registry->toSelfType("A"));

  auto nodeB = IdExprNode::MakeIdExprNode("b", 0, 0);
  context->symbolTable()->addElement("b", registry->toSelfType("B"));

  /// Successfull comparison expression between integers
  {
    auto node = BinaryExprNode<ComparisonOpID>::MakeBinaryExprNode(
        nodeI, nodeI, ComparisonOpID::Equal, 0, 0);
    auto status = typeCheckPass->visit(context.get(), node.get());
    ASSERT_TRUE(status.isOk());
    ASSERT_EQ(node->type(), registry->toType("Bool"));
  }

  /// Successfull comparison expression between non-integers
  {
    auto node = BinaryExprNode<ComparisonOpID>::MakeBinaryExprNode(
        nodeA, nodeA, ComparisonOpID::Equal, 0, 0);
    auto status = typeCheckPass->visit(context.get(), node.get());
    ASSERT_TRUE(status.isOk());
    ASSERT_EQ(node->type(), registry->toType("Bool"));
  }

  /// Successfull comparison expression between non-integers of different types
  {
    auto node = BinaryExprNode<ComparisonOpID>::MakeBinaryExprNode(
        nodeA, nodeB, ComparisonOpID::Equal, 0, 0);
    auto status = typeCheckPass->visit(context.get(), node.get());
    ASSERT_TRUE(status.isOk());
    ASSERT_EQ(node->type(), registry->toType("Bool"));
  }

  /// Left hand side and right hand side must be of same type
  {
    auto node = BinaryExprNode<ComparisonOpID>::MakeBinaryExprNode(
        nodeI, nodeA, ComparisonOpID::Equal, 0, 0);
    auto status = typeCheckPass->visit(context.get(), node.get());
    ASSERT_FALSE(status.isOk());

    /// Check error message
    auto *logger = GetLogger(context.get());
    ASSERT_EQ(logger->loggedMessageCount(), 1);
    ASSERT_EQ(logger->loggedMessage(0).message(),
              "Error: line 0, column 0. Equality comparison only possible "
              "between objects of the same type for Int, String and Bool. "
              "Types of objects compared are Int and A");
    logger->reset();
  }
}

/// BlockExprNode
TEST(TypeCheckTests, BlockExprNodeTests) {
  /// Create context
  auto context = MakeContextWithDefaultClasses();
  auto *registry = context->classRegistry();

  /// Create pass
  auto typeCheckPass = std::make_unique<TypeCheckPass>();

  /// Create nodes and register symbols
  auto nodeA = IdExprNode::MakeIdExprNode("a", 0, 0);
  auto nodeB = IdExprNode::MakeIdExprNode("b", 0, 0);
  context->symbolTable()->addElement("a", registry->toType("A"));
  context->symbolTable()->addElement("b", registry->toType("B"));

  /// Type of block expression is type of last expression
  {
    auto node = BlockExprNode::MakeBlockExprNode({nodeA, nodeB}, 0, 0);
    auto status = typeCheckPass->visit(context.get(), node.get());
    ASSERT_TRUE(status.isOk());
    ASSERT_EQ(node->type(), registry->toType("B"));
  }

  {
    auto node = BlockExprNode::MakeBlockExprNode({nodeB, nodeA}, 0, 0);
    auto status = typeCheckPass->visit(context.get(), node.get());
    ASSERT_TRUE(status.isOk());
    ASSERT_EQ(node->type(), registry->toType("A"));
  }
}

/// BooleanExprNode
TEST(TypeCheckTests, BooleanExprNodeTests) {
  /// Create context
  auto context = MakeContextWithDefaultClasses();
  auto *registry = context->classRegistry();

  /// Create pass
  auto typeCheckPass = std::make_unique<TypeCheckPass>();

  /// Type-check true expression
  {
    auto node = BooleanExprNode::MakeBooleanExprNode(true, 0, 0);
    auto status = typeCheckPass->visit(context.get(), node.get());
    ASSERT_TRUE(status.isOk());
    ASSERT_EQ(node->type(), registry->toType("Bool"));
  }

  /// Type-check false expression
  {
    auto node = BooleanExprNode::MakeBooleanExprNode(false, 0, 0);
    auto status = typeCheckPass->visit(context.get(), node.get());
    ASSERT_TRUE(status.isOk());
    ASSERT_EQ(node->type(), registry->toType("Bool"));
  }
}

/// CaseExprNode
TEST(TypeCheckTests, CaseExprNodeTests) {
  /// Create context
  auto context = MakeContextWithDefaultClasses();
  auto *registry = context->classRegistry();

  /// Create pass
  auto typeCheckPass = std::make_unique<TypeCheckPass>();

  /// Create nodes to be used in type-checking
  auto nodeA0 = IdExprNode::MakeIdExprNode("a0", 0, 0);
  auto nodeB0 = IdExprNode::MakeIdExprNode("b0", 0, 0);
  auto nodeD0 = IdExprNode::MakeIdExprNode("d0", 0, 0);
  context->symbolTable()->addElement("a0", registry->toType("A"));
  context->symbolTable()->addElement("b0", registry->toType("B"));
  context->symbolTable()->addElement("d0", registry->toType("D"));

  /// Create case node bindings
  auto nodeA = IdExprNode::MakeIdExprNode("a", 0, 0);
  auto nodeB = IdExprNode::MakeIdExprNode("b", 0, 0);
  auto nodeD = IdExprNode::MakeIdExprNode("d", 0, 0);
  auto bindingA = CaseBindingNode::MakeCaseBindingNode("a", "A", nodeA, 0, 0);
  auto bindingB = CaseBindingNode::MakeCaseBindingNode("b", "B", nodeB, 0, 0);
  auto bindingD = CaseBindingNode::MakeCaseBindingNode("d", "A", nodeD, 0, 0);

  /// Type-check expression in which return type is equal to node type
  {
    auto node =
        CaseExprNode::MakeCaseExprNode({bindingA, bindingB}, nodeA0, 0, 0);
    auto status = typeCheckPass->visit(context.get(), node.get());
    ASSERT_TRUE(status.isOk());
    ASSERT_EQ(node->type(), nodeA0->type());
  }

  /// Type-check expression in which return type is a subtype of node type
  {
    auto node =
        CaseExprNode::MakeCaseExprNode({bindingA, bindingB}, nodeB0, 0, 0);
    auto status = typeCheckPass->visit(context.get(), node.get());
    ASSERT_TRUE(status.isOk());
    ASSERT_EQ(node->type(), nodeA0->type());
  }

  /// Type-check fails because case expressions types are not unique
  {
    auto *logger = GetLogger(context.get());
    auto node =
        CaseExprNode::MakeCaseExprNode({bindingA, bindingD}, nodeA0, 0, 0);
    auto status = typeCheckPass->visit(context.get(), node.get());
    ASSERT_FALSE(status.isOk());
    ASSERT_EQ(logger->loggedMessageCount(), 1);
    ASSERT_EQ(
        logger->loggedMessage(0).message(),
        "Error: line 0, column 0. Types of case expressions must be unique");
    logger->reset();
  }

  /// Type-check fails because expression does not conform to any case type
  {
    auto *logger = GetLogger(context.get());
    auto node =
        CaseExprNode::MakeCaseExprNode({bindingA, bindingB}, nodeD0, 0, 0);
    auto status = typeCheckPass->visit(context.get(), node.get());
    ASSERT_FALSE(status.isOk());
    ASSERT_EQ(logger->loggedMessageCount(), 1);
    ASSERT_EQ(
        logger->loggedMessage(0).message(),
        "Error: line 0, column 0. Type of case expression does not conform "
        "to any case statement type");
    logger->reset();
  }
}

/// DispatchExprNode
TEST(TypeCheckTests, DispatchExprNode) {
  /// Create context
  auto context = MakeContextWithDefaultClasses();
  auto *registry = context->classRegistry();

  /// Create pass
  auto typeCheckPass = std::make_unique<TypeCheckPass>();

  /// Create caller node and register symbol
  auto nodeC = IdExprNode::MakeIdExprNode("z", 0, 0);
  context->symbolTable()->addElement("z", registry->toType("Z"));

  /// Create parameter nodes and register symbol
  auto nodeP1 = IdExprNode::MakeIdExprNode("p1", 0, 0);
  auto nodeP2 = IdExprNode::MakeIdExprNode("p2", 0, 0);
  context->symbolTable()->addElement("p1", registry->toType("B"));
  context->symbolTable()->addElement("p2", registry->toType("D"));

  /// Valid dispatch expression with SELF_TYPE return type
  {
    auto nodeM = DispatchExprNode::MakeDispatchExprNode("methodA", nodeC,
                                                        {nodeP1}, 0, 0);
    auto status = typeCheckPass->visit(context.get(), nodeM.get());
    ASSERT_TRUE(status.isOk());
    ASSERT_EQ(nodeM->type(), nodeC->type());
  }

  /// Valid dispatch expression with SELF_TYPE return type using this as
  /// dispatch object
  {
    context->setCurrentClassName("Z");
    auto nodeM = DispatchExprNode::MakeDispatchExprNode("methodA", nullptr,
                                                        {nodeP1}, 0, 0);
    auto status = typeCheckPass->visit(context.get(), nodeM.get());
    ASSERT_TRUE(status.isOk());
    ExprType expectedType{.typeID = nodeC->type().typeID, .isSelf = true};
    ASSERT_EQ(nodeM->type(), expectedType);
    context->setCurrentClassName("A");
  }

  /// Valid dispatch expression with return type other than SELF_TYPE
  {
    auto nodeM = DispatchExprNode::MakeDispatchExprNode("methodB", nodeC,
                                                        {nodeP2}, 0, 0);
    auto status = typeCheckPass->visit(context.get(), nodeM.get());
    ASSERT_TRUE(status.isOk());
    ASSERT_EQ(nodeM->type(), nodeP1->type());
  }

  /// Undefined method in dispatch object
  {
    auto *logger = GetLogger(context.get());
    auto nodeM = DispatchExprNode::MakeDispatchExprNode("methodC", nullptr,
                                                        {nodeP1}, 0, 0);
    auto status = typeCheckPass->visit(context.get(), nodeM.get());
    ASSERT_FALSE(status.isOk());
    ASSERT_EQ(logger->loggedMessageCount(), 1);
    ASSERT_EQ(logger->loggedMessage(0).message(),
              "Error: line 0, column 0. Method methodC of class A has not been "
              "defined");
    logger->reset();
  }

  /// Undefined method in caller object
  {
    auto *logger = GetLogger(context.get());
    auto nodeM = DispatchExprNode::MakeDispatchExprNode("methodC", nodeC,
                                                        {nodeP1}, 0, 0);
    auto status = typeCheckPass->visit(context.get(), nodeM.get());
    ASSERT_FALSE(status.isOk());
    ASSERT_EQ(logger->loggedMessageCount(), 1);
    ASSERT_EQ(logger->loggedMessage(0).message(),
              "Error: line 0, column 0. Method methodC of class Z has not been "
              "defined");
    logger->reset();
  }

  /// Undefined method in self dispatch object
  {
    auto *logger = GetLogger(context.get());
    auto nodeM = DispatchExprNode::MakeDispatchExprNode("methodC", nullptr,
                                                        {nodeP1}, 0, 0);
    auto status = typeCheckPass->visit(context.get(), nodeM.get());
    ASSERT_FALSE(status.isOk());
    ASSERT_EQ(logger->loggedMessageCount(), 1);
    ASSERT_EQ(logger->loggedMessage(0).message(),
              "Error: line 0, column 0. Method methodC of class A has not been "
              "defined");
    logger->reset();
  }

  /// Incorrect number of parameters
  {
    auto *logger = GetLogger(context.get());
    auto nodeM = DispatchExprNode::MakeDispatchExprNode("methodA", nodeC,
                                                        {nodeP1, nodeP2}, 0, 0);
    auto status = typeCheckPass->visit(context.get(), nodeM.get());
    ASSERT_FALSE(status.isOk());
    ASSERT_EQ(logger->loggedMessageCount(), 1);
    ASSERT_EQ(logger->loggedMessage(0).message(),
              "Error: line 0, column 0. Method methodA of class Z invoked with "
              "an invalid number of arguments. Expected: 1, actual: 2");
    logger->reset();
  }

  /// Incorrect number of parameters on self dispatch object
  {
    auto *logger = GetLogger(context.get());
    context->setCurrentClassName("Z");
    auto nodeM = DispatchExprNode::MakeDispatchExprNode("methodA", nullptr,
                                                        {nodeP1, nodeP2}, 0, 0);
    auto status = typeCheckPass->visit(context.get(), nodeM.get());
    ASSERT_FALSE(status.isOk());
    ASSERT_EQ(logger->loggedMessageCount(), 1);
    ASSERT_EQ(logger->loggedMessage(0).message(),
              "Error: line 0, column 0. Method methodA of class Z invoked with "
              "an invalid number of arguments. Expected: 1, actual: 2");
    logger->reset();
    context->setCurrentClassName("A");
  }

  /// Parameter of incorrect type
  {
    auto *logger = GetLogger(context.get());
    auto nodeM = DispatchExprNode::MakeDispatchExprNode("methodA", nodeC,
                                                        {nodeP2}, 0, 0);
    auto status = typeCheckPass->visit(context.get(), nodeM.get());
    ASSERT_FALSE(status.isOk());
    ASSERT_EQ(logger->loggedMessageCount(), 1);
    ASSERT_EQ(logger->loggedMessage(0).message(),
              "Error: line 0, column 0. Argument 1 of method methodA in class "
              "Z is of invalid type. Expected: B, actual: D");
    logger->reset();
  }

  /// Parameter of incorrect type on self dispatch object
  {
    context->setCurrentClassName("Z");
    auto *logger = GetLogger(context.get());
    auto nodeM = DispatchExprNode::MakeDispatchExprNode("methodA", nodeC,
                                                        {nodeP2}, 0, 0);
    auto status = typeCheckPass->visit(context.get(), nodeM.get());
    ASSERT_FALSE(status.isOk());
    ASSERT_EQ(logger->loggedMessageCount(), 1);
    ASSERT_EQ(logger->loggedMessage(0).message(),
              "Error: line 0, column 0. Argument 1 of method methodA in class "
              "Z is of invalid type. Expected: B, actual: D");
    logger->reset();
    context->setCurrentClassName("A");
  }
}

/// IdExprNode
TEST(TypeCheckTests, IdExprNodeTests) {
  /// Create context
  auto context = MakeContextWithDefaultClasses();
  auto *registry = context->classRegistry();

  /// Create pass
  auto typeCheckPass = std::make_unique<TypeCheckPass>();

  /// Create node and register symbol with dummy type
  auto nodeA = IdExprNode::MakeIdExprNode("a", 0, 0);
  context->symbolTable()->addElement("a", registry->toType("A"));

  /// Create second node without registering symbol
  auto nodeB = IdExprNode::MakeIdExprNode("b", 0, 0);

  /// Type-check of first node will succeed
  {
    auto status = typeCheckPass->visit(context.get(), nodeA.get());
    ASSERT_TRUE(status.isOk());
  }

  /// Type-check of second node will fail
  {
    auto status = typeCheckPass->visit(context.get(), nodeB.get());
    ASSERT_FALSE(status.isOk());

    /// Check error message
    auto *logger = GetLogger(context.get());
    ASSERT_EQ(logger->loggedMessageCount(), 1);
    ASSERT_EQ(logger->loggedMessage(0).message(),
              "Error: line 0, column 0. Variable b is not defined");
  }
}

/// IfExprNode
TEST(TypeCheckTests, IfExprNodeTests) {
  /// Create context
  auto context = MakeContextWithDefaultClasses();
  auto *classRegistry = context->classRegistry();

  /// Create pass
  auto typeCheckPass = std::make_unique<TypeCheckPass>();

  /// Create nodes for basic types
  auto nodeB = IdExprNode::MakeIdExprNode("bool", 0, 0);
  context->symbolTable()->addElement("bool", classRegistry->toType("Bool"));

  auto nodeI = IdExprNode::MakeIdExprNode("int", 0, 0);
  context->symbolTable()->addElement("int", classRegistry->toType("Int"));

  auto nodeS = IdExprNode::MakeIdExprNode("string", 0, 0);
  context->symbolTable()->addElement("string", classRegistry->toType("String"));

  /// Valid if expression. Expression type should be object
  {
    auto node = IfExprNode::MakeIfExprNode(nodeB, nodeI, nodeS, 0, 0);
    auto status = typeCheckPass->visit(context.get(), node.get());
    ASSERT_TRUE(status.isOk());
    ASSERT_EQ(node->type(), classRegistry->toType("Object"));
  }

  /// Valid if expression. Expression type should be String
  {
    auto node = IfExprNode::MakeIfExprNode(nodeB, nodeS, nodeS, 0, 0);
    auto status = typeCheckPass->visit(context.get(), node.get());
    ASSERT_TRUE(status.isOk());
    ASSERT_EQ(node->type(), classRegistry->toType("String"));
  }

  /// Valid if expression. Expression type should be Int
  {
    auto node = IfExprNode::MakeIfExprNode(nodeB, nodeI, nodeI, 0, 0);
    auto status = typeCheckPass->visit(context.get(), node.get());
    ASSERT_TRUE(status.isOk());
    ASSERT_EQ(node->type(), classRegistry->toType("Int"));
  }

  /// If condition is not of type Bool
  {
    auto node = IfExprNode::MakeIfExprNode(nodeS, nodeI, nodeI, 0, 0);
    auto status = typeCheckPass->visit(context.get(), node.get());
    ASSERT_FALSE(status.isOk());

    /// Check error message
    auto *logger = GetLogger(context.get());
    ASSERT_EQ(logger->loggedMessageCount(), 1);
    ASSERT_EQ(logger->loggedMessage(0).message(),
              "Error: line 0, column 0. Condition in if construct must be of "
              "Bool type. Actual type: String");
  }
}

/// UnaryExprNode (IsVoid)
TEST(TypeCheckTests, IsVoidExprNodeTests) {
  /// Create context
  auto context = MakeContextWithDefaultClasses();
  auto *registry = context->classRegistry();

  /// Create pass
  auto typeCheckPass = std::make_unique<TypeCheckPass>();

  /// Create nodes for basic types
  auto nodeA = IdExprNode::MakeIdExprNode("a", 0, 0);
  context->symbolTable()->addElement("a", registry->toType("A"));

  /// The type of an IsVoid expression is always Bool
  auto node = UnaryExprNode::MakeUnaryExprNode(nodeA, UnaryOpID::IsVoid, 0, 0);
  auto status = typeCheckPass->visit(context.get(), node.get());
  ASSERT_TRUE(status.isOk());
  ASSERT_EQ(node->type(), registry->toType("Bool"));
}

/// LetExprNode
TEST(TypeCheckTests, LetExprNodeTests) {
  /// Create context
  auto context = MakeContextWithDefaultClasses();
  auto *registry = context->classRegistry();

  /// Create pass
  auto typeCheckPass = std::make_unique<TypeCheckPass>();

  /// Create nodes and register symbols
  auto nodeA = IdExprNode::MakeIdExprNode("x1", 0, 0);
  auto nodeB = IdExprNode::MakeIdExprNode("x2", 0, 0);
  auto nodeC = IdExprNode::MakeIdExprNode("x3", 0, 0);
  context->symbolTable()->addElement("x1", registry->toType("Int"));

  auto nodeS1 = BinaryExprNode<ArithmeticOpID>::MakeBinaryExprNode(
      nodeA, nodeB, ArithmeticOpID::Plus, 0, 0);
  auto nodeS2 = BinaryExprNode<ArithmeticOpID>::MakeBinaryExprNode(
      nodeB, nodeC, ArithmeticOpID::Plus, 0, 0);

  /// Missing ID in binary expression defined in binding
  {
    auto letBindingNode =
        LetBindingNode::MakeLetBindingNode("x2", "Int", nodeA, 0, 0);
    auto letNode = LetExprNode::MakeLetExprNode({letBindingNode}, nodeS1, 0, 0);
    auto status = typeCheckPass->visit(context.get(), letNode.get());
    ASSERT_TRUE(status.isOk());
    ASSERT_EQ(letNode->type(), registry->toType("Int"));
  }

  /// Missing IDs in binary expression defined in bindings
  {
    auto letBindingNodeA =
        LetBindingNode::MakeLetBindingNode("x2", "Int", nodeA, 0, 0);
    auto letBindingNodeB =
        LetBindingNode::MakeLetBindingNode("x3", "Int", nodeA, 0, 0);
    auto letNode = LetExprNode::MakeLetExprNode(
        {letBindingNodeA, letBindingNodeB}, nodeS2, 0, 0);
    auto status = typeCheckPass->visit(context.get(), letNode.get());
    ASSERT_TRUE(status.isOk());
    ASSERT_EQ(letNode->type(), registry->toType("Int"));
  }

  /// ID in second binding defined in first binding
  {
    auto letBindingNodeA =
        LetBindingNode::MakeLetBindingNode("x3", "Int", nodeA, 0, 0);
    auto letBindingNodeB =
        LetBindingNode::MakeLetBindingNode("x2", "Int", nodeC, 0, 0);
    auto letNode = LetExprNode::MakeLetExprNode(
        {letBindingNodeA, letBindingNodeB}, nodeS1, 0, 0);
    auto status = typeCheckPass->visit(context.get(), letNode.get());
    ASSERT_TRUE(status.isOk());
    ASSERT_EQ(letNode->type(), registry->toType("Int"));
  }

  /// Incorrect bindings order
  {
    auto letBindingNodeA =
        LetBindingNode::MakeLetBindingNode("x3", "Int", nodeA, 0, 0);
    auto letBindingNodeB =
        LetBindingNode::MakeLetBindingNode("x2", "Int", nodeC, 0, 0);
    auto letNode = LetExprNode::MakeLetExprNode(
        {letBindingNodeB, letBindingNodeA}, nodeS1, 0, 0);
    auto status = typeCheckPass->visit(context.get(), letNode.get());
    ASSERT_FALSE(status.isOk());

    /// Check error message
    auto *logger = GetLogger(context.get());
    ASSERT_FALSE(status.isOk());
    ASSERT_EQ(logger->loggedMessageCount(), 1);
    ASSERT_EQ(logger->loggedMessage(0).message(),
              "Error: line 0, column 0. Variable x3 is not defined");
    logger->reset();
  }

  /// ID in binary expression not defined
  {
    auto letBindingNode =
        LetBindingNode::MakeLetBindingNode("x3", "Int", nodeA, 0, 0);
    auto letNode = LetExprNode::MakeLetExprNode({letBindingNode}, nodeS1, 0, 0);
    auto status = typeCheckPass->visit(context.get(), letNode.get());

    /// Check error message
    auto *logger = GetLogger(context.get());
    ASSERT_FALSE(status.isOk());
    ASSERT_EQ(logger->loggedMessageCount(), 1);
    ASSERT_EQ(logger->loggedMessage(0).message(),
              "Error: line 0, column 0. Variable x2 is not defined");
    logger->reset();
  }
}

/// LiteralExprNode
TEST(TypeCheckTests, LiteralExprNodeTests) {
  /// Create context
  auto context = MakeContextWithDefaultClasses();
  auto *registry = context->classRegistry();

  /// Create pass
  auto typeCheckPass = std::make_unique<TypeCheckPass>();

  /// Integer constant tests
  {
    auto node = LiteralExprNode<int32_t>::MakeLiteralExprNode(0, 0, 0);
    auto status = typeCheckPass->visit(context.get(), node.get());
    ASSERT_TRUE(status.isOk());
    ASSERT_EQ(node->type(), registry->toType("Int"));
  }

  /// String constant tests
  {
    auto node = LiteralExprNode<std::string>::MakeLiteralExprNode("", 0, 0);
    auto status = typeCheckPass->visit(context.get(), node.get());
    ASSERT_TRUE(status.isOk());
    ASSERT_EQ(node->type(), registry->toType("String"));
  }
}

/// NewExprNode
TEST(TypeCheckTests, NewExprNodeTests) {
  /// Create context
  auto context = MakeContextWithDefaultClasses();
  auto *registry = context->classRegistry();

  /// Create pass
  auto typeCheckPass = std::make_unique<TypeCheckPass>();

  /// Type exists
  {
    auto node = NewExprNode::MakeNewExprNode("A", 0, 0);
    auto status = typeCheckPass->visit(context.get(), node.get());
    ASSERT_TRUE(status.isOk());
    ASSERT_EQ(node->type(), registry->toType("A"));
  }

  /// Type is SELF_TYPE
  {
    auto node = NewExprNode::MakeNewExprNode("SELF_TYPE", 0, 0);
    auto status = typeCheckPass->visit(context.get(), node.get());
    ASSERT_TRUE(status.isOk());
    ASSERT_EQ(node->type(), registry->toSelfType("A"));
  }

  /// Type is not defined
  {
    auto node = NewExprNode::MakeNewExprNode("C", 0, 0);
    auto status = typeCheckPass->visit(context.get(), node.get());
    ASSERT_FALSE(status.isOk());

    /// Check error message
    auto *logger = GetLogger(context.get());
    ASSERT_EQ(logger->loggedMessageCount(), 1);
    ASSERT_EQ(
        logger->loggedMessage(0).message(),
        "Error: line 0, column 0. Type C in new expression is not defined");
    logger->reset();
  }
}

/// StaticDispatchExprNode
TEST(TypeCheckTests, StaticDispatchExprNode) {
  /// Create context
  auto context = MakeContextWithDefaultClasses();
  auto *registry = context->classRegistry();

  /// Create pass
  auto typeCheckPass = std::make_unique<TypeCheckPass>();

  /// Create caller node and register symbol
  auto nodeC = IdExprNode::MakeIdExprNode("x", 0, 0);
  context->symbolTable()->addElement("x", registry->toType("X"));

  /// Create parameter nodes and register symbol
  auto nodeP1 = IdExprNode::MakeIdExprNode("p1", 0, 0);
  auto nodeP2 = IdExprNode::MakeIdExprNode("p2", 0, 0);
  context->symbolTable()->addElement("p1", registry->toType("B"));
  context->symbolTable()->addElement("p2", registry->toType("D"));

  /// Valid static dispatch expression with SELF_TYPE return type. Dispatch type
  /// used is that of parent class
  {
    auto nodeM = StaticDispatchExprNode::MakeStaticDispatchExprNode(
        "methodA", "Z", nodeC, {nodeP1}, 0, 0);
    auto status = typeCheckPass->visit(context.get(), nodeM.get());
    ASSERT_TRUE(status.isOk());
    ASSERT_EQ(nodeM->type(), nodeC->type());
  }

  /// Valid static dispatch expression with SELF_TYPE return type. Dispatch type
  /// used is same as caller type
  {
    auto nodeM = StaticDispatchExprNode::MakeStaticDispatchExprNode(
        "methodA", "X", nodeC, {nodeP1}, 0, 0);
    auto status = typeCheckPass->visit(context.get(), nodeM.get());
    ASSERT_TRUE(status.isOk());
    ASSERT_EQ(nodeM->type(), nodeC->type());
  }

  /// Valid static dispatch expression with return type other than SELF_TYPE
  {
    auto nodeM = StaticDispatchExprNode::MakeStaticDispatchExprNode(
        "methodB", "Z", nodeC, {nodeP2}, 0, 0);
    auto status = typeCheckPass->visit(context.get(), nodeM.get());
    ASSERT_TRUE(status.isOk());
    ASSERT_EQ(nodeM->type(), nodeP1->type());
  }

  /// Caller type does not conform to dispatch type
  {
    auto *logger = GetLogger(context.get());
    auto nodeM = StaticDispatchExprNode::MakeStaticDispatchExprNode(
        "methodA", "B", nodeC, {nodeP1}, 0, 0);
    auto status = typeCheckPass->visit(context.get(), nodeM.get());
    ASSERT_FALSE(status.isOk());
    ASSERT_EQ(logger->loggedMessageCount(), 1);
    ASSERT_EQ(logger->loggedMessage(0).message(),
              "Error: line 0, column 0. Caller type X does not conform to "
              "dispatch type B");
    logger->reset();
  }

  /// Dispatch type is not defined
  {
    auto *logger = GetLogger(context.get());
    auto nodeM = StaticDispatchExprNode::MakeStaticDispatchExprNode(
        "methodA", "F", nodeC, {nodeP1}, 0, 0);
    auto status = typeCheckPass->visit(context.get(), nodeM.get());
    ASSERT_FALSE(status.isOk());
    ASSERT_EQ(logger->loggedMessageCount(), 1);
    ASSERT_EQ(logger->loggedMessage(0).message(),
              "Error: line 0, column 0. Dispatch type F is not defined");
    logger->reset();
  }
}

/// WhileExprNode
TEST(TypeCheckTests, WhileExprNodeTests) {
  /// Create context
  auto context = MakeContextWithDefaultClasses();
  auto *registry = context->classRegistry();

  /// Create pass
  auto typeCheckPass = std::make_unique<TypeCheckPass>();

  /// Create nodes for basic types
  auto nodeA = IdExprNode::MakeIdExprNode("a", 0, 0);
  context->symbolTable()->addElement("a", registry->toType("A"));

  auto nodeB = IdExprNode::MakeIdExprNode("bool", 0, 0);
  context->symbolTable()->addElement("bool", registry->toType("Bool"));

  /// Well-formed while loop
  {
    auto node = WhileExprNode::MakeWhileExprNode(nodeB, nodeA, 0, 0);
    auto status = typeCheckPass->visit(context.get(), node.get());
    ASSERT_TRUE(status.isOk());
    ASSERT_EQ(node->type(), registry->toType("Object"));
  }

  /// Loop condition is not of type Bool
  {
    auto node = WhileExprNode::MakeWhileExprNode(nodeA, nodeA, 0, 0);
    auto status = typeCheckPass->visit(context.get(), node.get());
    ASSERT_FALSE(status.isOk());

    /// Check error message
    auto *logger = GetLogger(context.get());
    ASSERT_EQ(logger->loggedMessageCount(), 1);
    ASSERT_EQ(logger->loggedMessage(0).message(),
              "Error: line 0, column 0. Loop condition must be of type Bool. "
              "Actual type: A");
    logger->reset();
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
