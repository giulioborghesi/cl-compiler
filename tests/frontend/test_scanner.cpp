#include <cool/frontend/scanner_extra.h>
#include <cool/frontend/scanner_state.h>

#include <cool/core/logger.h>
#include <cool/core/logger_collection.h>

#include <gtest/gtest.h>

#include <iostream>
#include <unistd.h>
#include <vector>

using namespace cool;

namespace {

/// Expected tokens sequence in lexer_test.cl
std::vector<uint32_t> TOKENS{CLASS_TOKEN,
                             CLASS_ID_TOKEN,
                             INHERITS_TOKEN,
                             CLASS_ID_TOKEN,
                             '{',
                             OBJECT_ID_TOKEN,
                             ':',
                             CLASS_ID_TOKEN,
                             ';',
                             OBJECT_ID_TOKEN,
                             '(',
                             OBJECT_ID_TOKEN,
                             ':',
                             CLASS_ID_TOKEN,
                             ')',
                             ':',
                             CLASS_ID_TOKEN,
                             '{',
                             '{',
                             OBJECT_ID_TOKEN,
                             ASSIGN_TOKEN,
                             OBJECT_ID_TOKEN,
                             ';',
                             OBJECT_ID_TOKEN,
                             ';',
                             '}',
                             '}',
                             ';'};

} // namespace

TEST(Scanner, BasicTests) {

  /// Parse input file. Tokens must match expected ones
  {
    const std::string filePath = "assets/lexer_test.cl";
    auto state = ScannerState::MakeFromFile(filePath);

    YYSTYPE yylval;
    YYLTYPE yylloc;
    for (auto expectedToken : TOKENS) {
      auto actualToken =
          yylex(&yylval, &yylloc, nullptr, state->scannerState());
      ASSERT_EQ(actualToken, expectedToken);
    }
  }

  /// Boolean literals
  {
    const std::string text = "true False tRue fAlSe True\0";
    auto state = ScannerState::MakeFromString(text);

    YYSTYPE yylval;
    YYLTYPE yylloc;

    /// First token recognized as boolean literal
    auto status = yylex(&yylval, &yylloc, nullptr, state->scannerState());
    ASSERT_EQ(status, TRUE_TOKEN);

    /// Second token recognized as class ID
    status = yylex(&yylval, &yylloc, nullptr, state->scannerState());
    ASSERT_EQ(status, CLASS_ID_TOKEN);

    /// Third and fourth token recognized as boolean literals
    status = yylex(&yylval, &yylloc, nullptr, state->scannerState());
    ASSERT_EQ(status, TRUE_TOKEN);

    status = yylex(&yylval, &yylloc, nullptr, state->scannerState());
    ASSERT_EQ(status, FALSE_TOKEN);

    /// Last token recognized as class ID
    status = yylex(&yylval, &yylloc, nullptr, state->scannerState());
    ASSERT_EQ(status, CLASS_ID_TOKEN);

    /// Next token is EOF
    status = yylex(&yylval, &yylloc, nullptr, state->scannerState());
    ASSERT_EQ(status, 0);
  }

  /// String
  {
    const std::string text = "\"Test special characters: \g \b\"";
    auto state = ScannerState::MakeFromString(text);

    YYSTYPE yylval;
    YYLTYPE yylloc;

    auto actualToken = yylex(&yylval, &yylloc, nullptr, state->scannerState());
    EXPECT_EQ(actualToken, STRING_TOKEN);

    const std::string actualText = yylval.literalVal;
    EXPECT_EQ(actualText, "Test special characters: g \b");
  }

  /// Inline comments
  {
    const std::string text = "-- One \nobjectName -- Two";
    auto state = ScannerState::MakeFromString(text);

    YYSTYPE yylval;
    YYLTYPE yylloc;

    /// Comments are ignored. First token recognized as object name
    auto status = yylex(&yylval, &yylloc, nullptr, state->scannerState());
    ASSERT_EQ(status, OBJECT_ID_TOKEN);

    /// Last token is EOF as comment extends to EOF
    status = yylex(&yylval, &yylloc, nullptr, state->scannerState());
    ASSERT_EQ(status, 0);
  }

  /// Out-of-line comments
  {
    const std::string text = "(* empty\n*)(*(**)empty*)object(*a\nempty\0";
    auto state = ScannerState::MakeFromString(text);

    YYSTYPE yylval;
    YYLTYPE yylloc;

    /// Comments are ignored First token recognized as object name
    auto status = yylex(&yylval, &yylloc, nullptr, state->scannerState());
    ASSERT_EQ(status, OBJECT_ID_TOKEN);

    /// Third token recognized as unterminated multi-line comment EOF
    status = yylex(&yylval, &yylloc, nullptr, state->scannerState());
    ASSERT_EQ(status, 0);
  }

  /// Invalid and valid characters
  {
    const std::string text = "\\n\n\0";
    auto state = ScannerState::MakeFromString(text);

    YYSTYPE yylval;
    YYLTYPE yylloc;

    /// Scanner skip the invalid character but pushes an error code
    auto status = yylex(&yylval, &yylloc, nullptr, state->scannerState());
    ASSERT_EQ(status, OBJECT_ID_TOKEN);
    ASSERT_EQ(state->lastErrorCode(),
              FrontEndErrorCode::LEXER_ERROR_INVALID_CHARACTER);

    /// Newline character should be eaten, next token is EOF
    state->resetErrorCode();
    status = yylex(&yylval, &yylloc, nullptr, state->scannerState());
    ASSERT_EQ(status, 0);
    ASSERT_EQ(state->lastErrorCode(), FrontEndErrorCode::NO_ERROR);
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
