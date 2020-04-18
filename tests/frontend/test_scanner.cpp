#include <cool/frontend/scanner_state.h>

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
      /*    FILE *inputFile = fopen("assets/lexer_test.cl", "r");
          ScannerState state = ScannerState::MakeFromFile(inputFile);

          YYSTYPE yylval;
          Scanner scanner{};
          for (auto expectedToken : TOKENS) {
            auto actualToken = scanner.nextToken(state.scannerState(), &yylval);
            ASSERT_EQ(actualToken.tokenID(), expectedToken);
          }

          fclose(inputFile); */
  }

  /// Boolean literals
  {
      /*    char stringLiterals[] = "true False tRue fAlSe True\0";
          yy_scan_buffer(stringLiterals, sizeof(stringLiterals));

          /// First token recognized as boolean literal
          auto status = yylex();
          ASSERT_EQ(status, TRUE_TOKEN);

          /// Second token recognized as class ID
          status = yylex();
          ASSERT_EQ(status, CLASS_ID_TOKEN);

          /// Third and fourth token recognized as boolean literals
          status = yylex();
          ASSERT_EQ(status, TRUE_TOKEN);

          status = yylex();
          ASSERT_EQ(status, FALSE_TOKEN);

          /// Last token recognized as class ID
          status = yylex();
          ASSERT_EQ(status, CLASS_ID_TOKEN);

          /// Next token is EOF
          status = yylex();
          ASSERT_EQ(status, EOF_TOKEN); */
  }

  /// Inline comments
  {
      /*    char stringComment[] = "-- One \nobjectName -- Two\0";
          yy_scan_buffer(stringComment, sizeof(stringComment));

          /// Comments are ignored. First token recognized as object name
          auto status = yylex();
          ASSERT_EQ(status, OBJECT_ID_TOKEN);

          /// Last token is EOF as comment extends to EOF
          status = yylex();
          ASSERT_EQ(status, EOF_TOKEN);*/
  }

  /// Out-of-line comments
  {
      /*    char stringComment[] = "(* empty\n
         *)(*(**)empty*)object(*a\nempty\0"; yy_scan_buffer(stringComment,
         sizeof(stringComment));

          /// Comments are ignored First token recognized as object name
          auto status = yylex();
          ASSERT_EQ(status, OBJECT_ID_TOKEN);

          /// Third token recognized as unterminated multi-line comment EOF
          status = yylex();
          ASSERT_EQ(status, SCANNER_ERROR_UNTERMINATED_COMMENT); */
  }

  /// Invalid and valid characters
  {
    /*    char stringInvalidChars[] = "\\n\n\0";
        yy_scan_buffer(stringInvalidChars, sizeof(stringInvalidChars));

        /// First token invalid - token starts with backslash
        auto status = yylex();
        ASSERT_EQ(status, SCANNER_ERROR_INVALID_CHARACTER);

        /// Second token recognized as identifier
        status = yylex();
        ASSERT_EQ(status, OBJECT_ID_TOKEN);

        /// Newline character should be eaten, next token is EOF
        status = yylex();
        ASSERT_EQ(status, EOF_TOKEN);*/
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
