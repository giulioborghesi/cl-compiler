%{

#include <cstdlib>
#include <string>
#include <unordered_map>

#include <cool/core/log_message.h>
#include <cool/frontend/error_codes.h>
#include <cool/frontend/scanner_extra.h>
#include <cool/frontend/scanner_spec.h>

/// Maximum buffer length
static constexpr size_t MAX_LENGTH = 1024;

/// Helper function to log an error message
void LogError(const cool::FrontEndErrorCode, const cool::ExtraState*, cool::LoggerCollection*);

/// Helper function to log the token location
void LogToken(const YYLTYPE*, const int32_t, cool::LoggerCollection*);

/// Helper function to update the token location 
void UpdateLocation(YYLTYPE*, struct cool::ExtraState*, const uint32_t);

%}

DIGIT  [0-9]

%option noyywrap reentrant bison-bridge bison-locations
%option extra-type="struct cool::ExtraState*"

%x STRING INLINECOMMENT COMMENT

%%

    /* Keywords */
(?i:case)               { 
                            UpdateLocation(yylloc, yyextra, 4); 
                            LogToken(yylloc, CASE_TOKEN, logger); 
                            return CASE_TOKEN; 
                        }
(?i:class)              { 
                            UpdateLocation(yylloc, yyextra, 5); 
                            LogToken(yylloc, CLASS_TOKEN, logger); 
                            return CLASS_TOKEN; 
                        }
(?i:else)               { 
                            UpdateLocation(yylloc, yyextra, 4); 
                            LogToken(yylloc, ELSE_TOKEN, logger); 
                            return ELSE_TOKEN; 
                        }
(?i:esac)               { 
                            UpdateLocation(yylloc, yyextra, 4); 
                            LogToken(yylloc, ESAC_TOKEN, logger);
                            return ESAC_TOKEN; 
                        }
f(?i:alse)              { 
                            UpdateLocation(yylloc, yyextra, 5); 
                            LogToken(yylloc, FALSE_TOKEN, logger); 
                            return FALSE_TOKEN; 
                        }
(?i:fi)                 { 
                            UpdateLocation(yylloc, yyextra, 2); 
                            LogToken(yylloc, FI_TOKEN, logger); 
                            return FI_TOKEN; 
                        }
(?i:if)                 { 
                            UpdateLocation(yylloc, yyextra, 2); 
                            LogToken(yylloc, IF_TOKEN, logger); 
                            return IF_TOKEN; 
                        }
(?i:in)                 { 
                            UpdateLocation(yylloc, yyextra, 2); 
                            LogToken(yylloc, IN_TOKEN, logger);
                            return IN_TOKEN; 
                        }
(?i:inherits)           { 
                            UpdateLocation(yylloc, yyextra, 8); 
                            LogToken(yylloc, INHERITS_TOKEN, logger);
                            return INHERITS_TOKEN; 
                        }
(?i:isvoid)             { 
                            UpdateLocation(yylloc, yyextra, 6); 
                            LogToken(yylloc, ISVOID_TOKEN, logger);
                            return ISVOID_TOKEN; 
                        }
(?i:let)                { 
                            UpdateLocation(yylloc, yyextra, 3); 
                            LogToken(yylloc, LET_TOKEN, logger);
                            return LET_TOKEN; 
                        }
(?i:loop)               { 
                            UpdateLocation(yylloc, yyextra, 4); 
                            LogToken(yylloc, LOOP_TOKEN, logger);
                            return LOOP_TOKEN; 
                        }
(?i:new)                { 
                            UpdateLocation(yylloc, yyextra, 3); 
                            LogToken(yylloc, NEW_TOKEN, logger);
                            return NEW_TOKEN; 
                        }
(?i:not)                { 
                            UpdateLocation(yylloc, yyextra, 3); 
                            LogToken(yylloc, NOT_TOKEN, logger);
                            return NOT_TOKEN; 
                        }
(?i:of)                 { 
                            UpdateLocation(yylloc, yyextra, 2); 
                            LogToken(yylloc, OF_TOKEN, logger);
                            return OF_TOKEN; 
                        }
(?i:pool)               { 
                            UpdateLocation(yylloc, yyextra, 4); 
                            LogToken(yylloc, POOL_TOKEN, logger);
                            return POOL_TOKEN; 
                        }
(?i:then)               { 
                            UpdateLocation(yylloc, yyextra, 4); 
                            LogToken(yylloc, THEN_TOKEN, logger);
                            return THEN_TOKEN; 
                        }
t(?i:rue)               { 
                            UpdateLocation(yylloc, yyextra, 4); 
                            LogToken(yylloc, TRUE_TOKEN, logger);
                            return TRUE_TOKEN; 
                        }
(?i:while)              { 
                            UpdateLocation(yylloc, yyextra, 5); 
                            LogToken(yylloc, WHILE_TOKEN, logger);
                            return WHILE_TOKEN; 
                        }

    /* Assignment operator */
"<-"                    { 
                            UpdateLocation(yylloc, yyextra, 2); 
                            LogToken(yylloc, ASSIGN_TOKEN, logger);
                            return ASSIGN_TOKEN; 
                        }

    /* Case operator */
"=>"                    { 
                            UpdateLocation(yylloc, yyextra, 2); 
                            LogToken(yylloc, CASE_OPERATOR_TOKEN, logger);
                            return CASE_OPERATOR_TOKEN; 
                        }

    /* In-line comment */
"--"                    {   BEGIN(INLINECOMMENT);   }
<INLINECOMMENT>[\n]     {   BEGIN(INITIAL); yyextra->currentLine++; yyextra->currentColumn = 1;   }
<INLINECOMMENT>.        {   /* Nothing to do */   }
<INLINECOMMENT><<EOF>>  {   BEGIN(INITIAL); return 0;   }

    /* Out-of-line comment */
"(\*"                   {   BEGIN(COMMENT); yyextra->openComments = 1;   }
<COMMENT>"(\*"          {   yyextra->openComments += 1; }
<COMMENT>"\*)"          { 
                            yyextra->openComments -= 1; 
                            if (yyextra->openComments == 0) { 
                              BEGIN(INITIAL); 
                            } 
                        }
<COMMENT>[\n]           {   yyextra->currentLine++; yyextra->currentColumn = 1;   }
<COMMENT>.              {   yyextra->currentColumn++; }
<COMMENT><<EOF>>        { 
                            BEGIN(INITIAL); 
                            LogError(cool::FrontEndErrorCode::LEXER_ERROR_UNTERMINATED_COMMENT, yyextra, logger); 
                            yyextra->lastErrorCode = cool::FrontEndErrorCode::LEXER_ERROR_UNTERMINATED_COMMENT;
                        }

    /* Single character tokens */
"+" |
"-" |
"*" |
"/" |
"<" |
"=" |
"~" |
"(" |
")" |
"{" |
"}" |
"@" | 
"." |
":" |
";" |
","                     { 
                            UpdateLocation(yylloc, yyextra, 1); 
                            LogToken(yylloc, yytext[0], logger);
                            return yytext[0]; 
                        }

    /* Comparison operators */
"<="                    { 
                            UpdateLocation(yylloc, yyextra, 2); 
                            LogToken(yylloc, LESS_EQUAL_TOKEN, logger);
                            return LESS_EQUAL_TOKEN; 
                        }

    /* Integers */
{DIGIT}+                { 
                            UpdateLocation(yylloc, yyextra, yyleng);
                            yylval->integerVal = atoi(yytext);
                            if (logger) {
                                logger->logMessage(cool::LogMessage::MakeDebugMessage(
                                    "line: %d, col: %d: INTEGER_VAL: %d", 
                                    yylloc->first_line, 
                                    yylloc->first_column,
                                    yylval->integerVal)); 
                            }
                            return INTEGER_TOKEN; 
                        }

    /* Identifiers */
[A-Z][a-zA-Z0-9_]*      { 
                            UpdateLocation(yylloc, yyextra, yyleng);
                            yylval->literalVal = strdup(yytext); 
                            if (logger) {
                                logger->logMessage(cool::LogMessage::MakeDebugMessage(
                                    "line: %d, col: %d: CLASS_ID: %s", 
                                    yylloc->first_line,
                                    yylloc->first_column,
                                    yylval->literalVal));
                            }
                            return CLASS_ID_TOKEN; 
                        }  
[a-z][a-zA-Z0-9_]*      { 
                            UpdateLocation(yylloc, yyextra, yyleng);
                            yylval->literalVal = strdup(yytext); 
                            if (logger) {                            
                                logger->logMessage(cool::LogMessage::MakeDebugMessage(
                                    "line: %d, col: %d: OBJECT_ID: %s", 
                                    yylloc->first_line, 
                                    yylloc->first_column,
                                    yylval->literalVal));
                            }
                            return OBJECT_ID_TOKEN; 
                        }

    /* White spaces and newlines */
"\n"                    {   yyextra->currentLine++; yyextra->currentColumn = 1;   }
[ \f\r\t\v]             {   yyextra->currentColumn++;   }

   /* Strings */
"\""                    { 
                            yyextra->stringText.clear(); 
                            yylloc->first_line = yyextra->currentLine;
                            yylloc->first_column = yyextra->currentColumn;
                            yyextra->currentColumn++;
                            BEGIN(STRING); 
                        }
<STRING>"\""            { 
                            yylloc->last_line = yyextra->currentLine;
                            yylloc->last_column = yyextra->currentColumn;
                            BEGIN(INITIAL); 
                            yylval->literalVal = yyextra->stringText;
                            if (yylval->literalVal.length() > MAX_LENGTH) { 
                                LogError(cool::FrontEndErrorCode::LEXER_ERROR_STRING_EXCEEDS_MAX_LENGTH, yyextra, logger); 
                                yyextra->lastErrorCode = cool::FrontEndErrorCode::LEXER_ERROR_STRING_EXCEEDS_MAX_LENGTH;
                                yyextra->currentColumn++;
                            } else {
                                if (logger) {
                                    logger->logMessage(cool::LogMessage::MakeDebugMessage(
                                        "line: %d, col: %d: STRING: %s", 
                                        yylloc->first_line, 
                                        yylloc->first_column,
                                        yylval->literalVal));
                                }
                                yyextra->currentColumn++;
                                return STRING_TOKEN;
                            }
                        }
<STRING>"\\\n"          { 
                            yyextra->currentLine++;
                            yyextra->currentColumn = 1;
                            yyextra->stringText.append("\\\\n"); 
                        }
<STRING>"\\n"           {  
                            yyextra->currentColumn += 2;
                            yyextra->stringText.append("\\n"); 
                        }
<STRING>"\n"            { 
                            LogError(cool::FrontEndErrorCode::LEXER_ERROR_STRING_CONTAINS_NEWLINE_CHARACTER, yyextra, logger);
                            yyextra->lastErrorCode = cool::FrontEndErrorCode::LEXER_ERROR_STRING_CONTAINS_NEWLINE_CHARACTER;
                            yyextra->currentLine++;
                            yyextra->currentColumn = 1;
                            BEGIN(INITIAL); 
                        }
<STRING>"\0"            { 
                            LogError(cool::FrontEndErrorCode::LEXER_ERROR_STRING_CONTAINS_NULL_CHARACTER, yyextra, logger);
                            yyextra->lastErrorCode = cool::FrontEndErrorCode::LEXER_ERROR_STRING_CONTAINS_NULL_CHARACTER;
                            yyextra->currentColumn++;
                            BEGIN(INITIAL); 
                        }
<STRING>"\\0"           { 
                            yyextra->currentColumn += 2;
                            yyextra->stringText.append("\\0"); 
                        }
<STRING>"\\b"           { 
                            yyextra->currentColumn += 2;
                            yyextra->stringText.append("\\b"); 
                        }
<STRING>"\\t"           { 
                            yyextra->currentColumn += 2;
                            yyextra->stringText.append("\\t"); 
                        }
<STRING>"\\f"           {
                            yyextra->currentColumn += 2; 
                            yyextra->stringText.append("\\f"); 
                        }
<STRING>"\\"[^btf]      { 
                            yyextra->currentColumn += 2;
                            yyextra->stringText.push_back(yytext[1]); 
                        }
<STRING>.               {
                            yyextra->currentColumn += 1; 
                            yyextra->stringText.push_back(yytext[0]); 
                        }
<STRING><<EOF>>         { 
                            LogError(cool::FrontEndErrorCode::LEXER_ERROR_UNTERMINATED_STRING, yyextra, logger); 
                            yyextra->lastErrorCode = cool::FrontEndErrorCode::LEXER_ERROR_UNTERMINATED_STRING;
                            BEGIN(INITIAL);
                            return 0; 
                        }

   /* End of file */
<<EOF>>                 {   return 0;   }

   /* All other characters are invalid and should trigger an error */
.                       { 
                            LogError(cool::FrontEndErrorCode::LEXER_ERROR_INVALID_CHARACTER, yyextra, logger);
                            yyextra->lastErrorCode = cool::FrontEndErrorCode::LEXER_ERROR_INVALID_CHARACTER;
                            yyextra->currentColumn++; 
                        }
                            
%%

/// Helper function to log an error message
void LogError(const cool::FrontEndErrorCode code, const cool::ExtraState* extraState, cool::LoggerCollection* logger) {

    static std::unordered_map<cool::FrontEndErrorCode, std::string> sErrorToString = {
        {cool::FrontEndErrorCode::LEXER_ERROR_UNTERMINATED_COMMENT, "unterminated comment"},
        {cool::FrontEndErrorCode::LEXER_ERROR_STRING_EXCEEDS_MAX_LENGTH, "string exceeds maximum length"},
        {cool::FrontEndErrorCode::LEXER_ERROR_STRING_CONTAINS_NEWLINE_CHARACTER, "unescaped newline character in string is not allowed"},
        {cool::FrontEndErrorCode::LEXER_ERROR_STRING_CONTAINS_NULL_CHARACTER, "unescaped null character in string is not allowed"},
        {cool::FrontEndErrorCode::LEXER_ERROR_UNTERMINATED_STRING, "unterminated string"},
        {cool::FrontEndErrorCode::LEXER_ERROR_INVALID_CHARACTER, "invalid character in input stream"}
    };

    /// Do nothing if logger is not registered
    if (!logger) {
        return;
    }

    /// Guard against unexpected errors
    assert(sErrorToString.count(code) > 0);

    /// Log error message
    logger->logMessage(cool::LogMessage::MakeErrorMessage("line: %d, col: %d: Error: %s", 
        extraState->currentLine, extraState->currentColumn, sErrorToString[code]));
}

void LogToken(const YYLTYPE* loc, const int32_t tokenCode, cool::LoggerCollection* logger) { 

    /// Dictionary mapping a token code to its textual representation
    static std::unordered_map<int32_t, std::string> sTokenToString = {
        {CASE_TOKEN, "CASE_KEYWORD"}, {CLASS_TOKEN, "CLASS_KEYWORD"}, {ELSE_TOKEN, "ELSE_KEYWORD"},
        {ESAC_TOKEN, "ESAC_KEYWORD"}, {FALSE_TOKEN, "FALSE_KEYWORD"}, {FI_TOKEN, "FI_KEYWORD"},
        {IF_TOKEN, "IF_KEYWORD"}, {IN_TOKEN, "IN_KEYWORD"}, {INHERITS_TOKEN, "INHERITS_KEYWORD"}, 
        {ISVOID_TOKEN, "ISVOID_KEYWORD"}, {LET_TOKEN, "LET_KEYWORD"}, {LOOP_TOKEN, "LOOP_KEYWORD"},
        {NEW_TOKEN, "NEW_KEYWORD"}, {NOT_TOKEN, "NOT_KEYWORD"}, {OF_TOKEN, "OF_KEYWORD"},
        {POOL_TOKEN, "POOL_KEYWORD"}, {THEN_TOKEN, "THEN_KEYWORD"}, {TRUE_TOKEN, "TRUE_KEYWORD"},
        {WHILE_TOKEN, "WHILE_KEYWORD"}, {ASSIGN_TOKEN, "<-"}, {CASE_OPERATOR_TOKEN, "=>"}, 
        {LESS_EQUAL_TOKEN, "<="}
    };

    /// Do nothing if logger is not registered
    if (!logger) {
        return;
    }

    /// Get the token text
    const std::string tokenText = sTokenToString.count(tokenCode) ? 
        sTokenToString[tokenCode] : std::string(1, (char)tokenCode);
    
    /// Log token
    logger->logMessage(cool::LogMessage::MakeDebugMessage("line: %d, col: %d: %s", 
        loc->first_line, loc->first_column, tokenText));
}

void UpdateLocation(YYLTYPE* loc, cool::ExtraState* extraState, const uint32_t length) {
    loc->first_line = loc->last_line = extraState->currentLine;
    loc->first_column = extraState->currentColumn;

    extraState->currentColumn += length;
    loc->last_column = extraState->currentColumn - 1;
}
