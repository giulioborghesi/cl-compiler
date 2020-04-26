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
}

TEST(Parser, InvalidAttribute) {
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

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
