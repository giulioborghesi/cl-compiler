//#include <cool/frontend/scanner_state.h>
#include <cool/frontend/parser.h>
#include <cool/ir/class.h>

#include <gtest/gtest.h>

#include <iostream>
#include <string>
#include <vector>

using namespace cool;

TEST(Parser, BasicTest) {
  /// Create parser
  const std::string filePath = "assets/parser_test.cl";
  auto parser = Parser::MakeFromFile(filePath);

  /// Parse program and verify results
  auto programNode = parser.parse();
  ASSERT_NE(programNode, nullptr);
  ASSERT_EQ(programNode->classes().size(), 1);
  ASSERT_EQ(programNode->classes()[0]->className(), "CellularAutomaton");
  ASSERT_EQ(programNode->classes()[0]->attributes().size(), 2);
}

TEST(Parser, InvalidVariableAttribute) {
  /// Create parser
  const std::string programText = "class Test {\n attr String; \n };";
  auto parser = Parser::MakeFromString(programText);

  /// Parse program
  auto programNode = parser.parse();
  ASSERT_EQ(parser.lastErrorCode(),
            FrontEndErrorCode::PARSER_ERROR_INVALID_FEATURE);

  /// Verify results
  ASSERT_NE(programNode, nullptr);
  ASSERT_EQ(programNode->classes().size(), 1);
  ASSERT_EQ(programNode->classes()[0]->className(), "Test");
  ASSERT_EQ(programNode->classes()[0]->attributes().size(), 0);
}

TEST(Parser, InvalidMethodAttribute) {
  /// Create parser
  const std::string programText =
      "class AttrTest {\n init(string string); attr : String; \n };";
  auto parser = Parser::MakeFromString(programText);

  /// Parse program
  auto programNode = parser.parse();
  ASSERT_EQ(parser.lastErrorCode(),
            FrontEndErrorCode::PARSER_ERROR_INVALID_FEATURE);

  /// Verify results
  ASSERT_NE(programNode, nullptr);
  ASSERT_EQ(programNode->classes().size(), 1);
  ASSERT_EQ(programNode->classes()[0]->className(), "AttrTest");
  ASSERT_EQ(programNode->classes()[0]->attributes().size(), 1);
}

TEST(Parser, InvalidExpressionInBlock) {
  /// Create parser
  const std::string programText =
      "class ExprTest { init(map : String) : SELF_TYPE { {a; b : IO; } }; };";
  auto parser = Parser::MakeFromString(programText);

  /// Parse program
  auto programNode = parser.parse();
  ASSERT_EQ(parser.lastErrorCode(),
            FrontEndErrorCode::PARSER_ERROR_INVALID_EXPRESSION);

  /// Verify results
  ASSERT_NE(programNode, nullptr);
  ASSERT_EQ(programNode->classes().size(), 1);
  ASSERT_EQ(programNode->classes()[0]->className(), "ExprTest");
  ASSERT_EQ(programNode->classes()[0]->attributes().size(), 1);

  auto formalAttributeNode = programNode->classes()[0]->attributes()[0].get();
  auto methodNode = dynamic_cast<MethodNode *>(formalAttributeNode);
  ASSERT_NE(methodNode, nullptr);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
