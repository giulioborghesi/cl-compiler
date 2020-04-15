#include <cool/frontend/scanner.h>
#include <cool/frontend/scanner_spec.h>

#include <gtest/gtest.h>

#include <iostream>
#include <unistd.h>
#include <vector>

typedef struct yy_buffer_state *YY_BUFFER_STATE;
extern int yylex();
extern YY_BUFFER_STATE yy_scan_buffer(char *, size_t);

YYSTYPE yylval;

using namespace cool;

namespace {

/// Expected tokens sequence in lexer_test.cl
std::vector<uint32_t> TOKENS{TOKEN_CLASS,
                             TOKEN_CLASS_ID,
                             TOKEN_INHERITS,
                             TOKEN_CLASS_ID,
                             TOKEN_LEFT_CURLY_BRACE,
                             TOKEN_OBJECT_ID,
                             TOKEN_COLUMN,
                             TOKEN_CLASS_ID,
                             TOKEN_SEMICOLUMN,
                             TOKEN_OBJECT_ID,
                             TOKEN_LEFT_PARENTHESIS,
                             TOKEN_OBJECT_ID,
                             TOKEN_COLUMN,
                             TOKEN_CLASS_ID,
                             TOKEN_RIGHT_PARENTHESIS,
                             TOKEN_COLUMN,
                             TOKEN_CLASS_ID,
                             TOKEN_LEFT_CURLY_BRACE,
                             TOKEN_LEFT_CURLY_BRACE,
                             TOKEN_OBJECT_ID,
                             TOKEN_ASSIGN,
                             TOKEN_OBJECT_ID,
                             TOKEN_SEMICOLUMN,
                             TOKEN_OBJECT_ID,
                             TOKEN_SEMICOLUMN,
                             TOKEN_RIGHT_CURLY_BRACE,
                             TOKEN_RIGHT_CURLY_BRACE,
                             TOKEN_SEMICOLUMN};

} // namespace

TEST(Scanner, BasicTests) {

  /// Parse input file. Tokens must match expected ones
  {
    Scanner scanner;
    auto statusInput = scanner.setInputFile("assets/lexer_test.cl");
    ASSERT_TRUE(statusInput.isOk());

    for (auto expectedToken : TOKENS) {
      auto actualToken = scanner.nextToken();
      ASSERT_EQ(actualToken.tokenID, expectedToken);
    }
  }

  /// Boolean literals
  {
    char stringLiterals[] = "true False tRue fAlSe True\0";
    yy_scan_buffer(stringLiterals, sizeof(stringLiterals));

    /// First token recognized as boolean literal
    auto status = yylex();
    ASSERT_EQ(status, TOKEN_TRUE);

    /// Second token recognized as class ID
    status = yylex();
    ASSERT_EQ(status, TOKEN_CLASS_ID);

    /// Third and fourth token recognized as boolean literals
    status = yylex();
    ASSERT_EQ(status, TOKEN_TRUE);

    status = yylex();
    ASSERT_EQ(status, TOKEN_FALSE);

    /// Last token recognized as class ID
    status = yylex();
    ASSERT_EQ(status, TOKEN_CLASS_ID);

    /// Next token is EOF
    status = yylex();
    ASSERT_EQ(status, TOKEN_EOF);
  }

  /// Inline comments
  {
    char stringComment[] = "-- One \nobjectName -- Two\0";
    yy_scan_buffer(stringComment, sizeof(stringComment));

    /// Comments are ignored. First token recognized as object name
    auto status = yylex();
    ASSERT_EQ(status, TOKEN_OBJECT_ID);

    /// Last token is EOF as comment extends to EOF
    status = yylex();
    ASSERT_EQ(status, TOKEN_EOF);
  }

  /// Out-of-line comments
  {
    char stringComment[] = "(* empty\n *)(*(**)empty*)object(*a\nempty\0";
    yy_scan_buffer(stringComment, sizeof(stringComment));

    /// Comments are ignored First token recognized as object name
    auto status = yylex();
    ASSERT_EQ(status, TOKEN_OBJECT_ID);

    /// Third token recognized as unterminated multi-line comment EOF
    status = yylex();
    ASSERT_EQ(status, SCANNER_ERROR_UNTERMINATED_COMMENT);
  }

  /// Invalid and valid characters
  {
    char stringInvalidChars[] = "\\n\n\0";
    yy_scan_buffer(stringInvalidChars, sizeof(stringInvalidChars));

    /// First token invalid - token starts with backslash
    auto status = yylex();
    ASSERT_EQ(status, SCANNER_ERROR_INVALID_CHARACTER);

    /// Second token recognized as identifier
    status = yylex();
    ASSERT_EQ(status, TOKEN_OBJECT_ID);

    /// Newline character should be eaten, next token is EOF
    status = yylex();
    ASSERT_EQ(status, TOKEN_EOF);
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
