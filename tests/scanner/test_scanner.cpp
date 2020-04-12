#include <cool/scanner/cool_flex.h>

#include <gtest/gtest.h>

#include <iostream>

typedef struct yy_buffer_state *YY_BUFFER_STATE;
extern int yylex();
extern YY_BUFFER_STATE yy_scan_buffer(char *, size_t);

YYSTYPE yylval;

TEST(Scanner, BasicTests) {

  /// Case-insensitive class keyword
  {
    char stringKeywords[] = "class cLasS ClaSS CLASS\0";
    yy_scan_buffer(stringKeywords, sizeof(stringKeywords));
    for (uint32_t i = 0; i < 4; ++i) {
      auto status = yylex();
      ASSERT_EQ(status, TOKEN_CLASS);
    }

    auto status = yylex();
    ASSERT_EQ(status, TOKEN_EOF);
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

    /// Third token recognized as unterminated multi-line comment
    status = yylex();
    ASSERT_EQ(status, -5);
  }

  /// Invalid and valid characters
  {
    char stringInvalidChars[] = "\\n\n\0";
    yy_scan_buffer(stringInvalidChars, sizeof(stringInvalidChars));

    /// First token invalid - token starts with backslash
    auto status = yylex();
    ASSERT_EQ(status, -2);

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
