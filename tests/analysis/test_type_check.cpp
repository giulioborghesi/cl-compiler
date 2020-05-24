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

  /// Set A to current class and return
  context->setCurrentClassName("A");
  return context;
}

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
