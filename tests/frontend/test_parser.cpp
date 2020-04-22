#include <cool/frontend/scanner_state.h>
#include <cool/ir/class.h>

#include <gtest/gtest.h>

#include <iostream>
#include <string>
#include <vector>

using namespace cool;

TEST(Parser, BasicTest) {
  const std::string filePath = "assets/parser_test.cl";
  ScannerState state = ScannerState::MakeFromFile(filePath);

  ProgramNodePtr programNode;
  auto statusParse = yyparse(nullptr, state.scannerState(), &programNode);

  ASSERT_NE(programNode, nullptr);
  ASSERT_EQ(programNode->classes().size(), 1);
  ASSERT_EQ(programNode->classes()[0]->className(), "CellularAutomaton");
  ASSERT_EQ(statusParse, 0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
