%{

#include <cstdlib>
#include <string>

#include <cool/frontend/scanner_extra.h>
#include <cool/frontend/scanner_spec.h>

/// Maximum buffer length
static constexpr size_t MAX_STRING_LENGTH = 1024;

%}

DIGIT  [0-9]

%option noyywrap reentrant bison-bridge bison-locations
%option extra-type="struct cool::ExtraState*"

%x STRING INLINECOMMENT COMMENT

void UpdateLocation(YYLTYPE*, struct cool::ExtraState*, const uint32_t);

%%

    /* Keywords */
(?i:case)              { UpdateLocation(yylloc, yyextra, 4); return CASE_TOKEN; }
(?i:class)             { UpdateLocation(yylloc, yyextra, 5); return CLASS_TOKEN; }
(?i:else)              { UpdateLocation(yylloc, yyextra, 4); return ELSE_TOKEN; }
f(?i:alse)             { UpdateLocation(yylloc, yyextra, 5); return FALSE_TOKEN; }
(?i:fi)                { UpdateLocation(yylloc, yyextra, 2); return FI_TOKEN; }
(?i:if)                { UpdateLocation(yylloc, yyextra, 2); return IF_TOKEN; }
(?i:in)                { UpdateLocation(yylloc, yyextra, 2); return IN_TOKEN; }
(?i:inherits)          { UpdateLocation(yylloc, yyextra, 8); return INHERITS_TOKEN; }
(?i:isvoid)            { UpdateLocation(yylloc, yyextra, 6); return ISVOID_TOKEN; }
(?i:let)               { UpdateLocation(yylloc, yyextra, 3); return LET_TOKEN; }
(?i:loop)              { UpdateLocation(yylloc, yyextra, 4); return LOOP_TOKEN; }
(?i:new)               { UpdateLocation(yylloc, yyextra, 3); return NEW_TOKEN; }
(?i:not)               { UpdateLocation(yylloc, yyextra, 3); return NOT_TOKEN; }
(?i:pool)              { UpdateLocation(yylloc, yyextra, 4); return POOL_TOKEN; }
(?i:then)              { UpdateLocation(yylloc, yyextra, 4); return THEN_TOKEN; }
t(?i:rue)              { UpdateLocation(yylloc, yyextra, 4); return TRUE_TOKEN; }
(?i:while)             { UpdateLocation(yylloc, yyextra, 5); return WHILE_TOKEN; }
(?i:esac)              { UpdateLocation(yylloc, yyextra, 4); return ESAC_TOKEN; }
(?i:of)                { UpdateLocation(yylloc, yyextra, 2); return OF_TOKEN; }

    /* Assignment operator */
"<-"                   { UpdateLocation(yylloc, yyextra, 2); return ASSIGN_TOKEN; }

    /* Case operator */
"=>"                   { UpdateLocation(yylloc, yyextra, 2); return CASE_OPERATOR_TOKEN; }

    /* In-line comment */
"--"                   { BEGIN(INLINECOMMENT); }
<INLINECOMMENT>[\n]    { BEGIN(INITIAL); yyextra->currentLine++; yyextra->currentColumn = 1; }
<INLINECOMMENT>.       { }
<INLINECOMMENT><<EOF>> { BEGIN(INITIAL); return 0; }

    /* Out-of-line comment */
"(\*"                  { BEGIN(COMMENT); yyextra->openComments = 1; }
<COMMENT>"(\*"         { yyextra->openComments += 1;}
<COMMENT>"\*)"         { yyextra->openComments -= 1; if (yyextra->openComments == 0) { BEGIN(INITIAL); }}
<COMMENT>[\n]          { yyextra->currentLine++; yyextra->currentColumn = 1; }
<COMMENT>.             { yyextra->currentColumn++; }
<COMMENT><<EOF>>       { BEGIN(INITIAL); return SCANNER_ERROR_UNTERMINATED_COMMENT; }

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
","                    { UpdateLocation(yylloc, yyextra, 1); return yytext[0]; }

    /* Comparison operators */
"<="                   { UpdateLocation(yylloc, yyextra, 2); return LESS_EQUAL_TOKEN; }

    /* Integers */
{DIGIT}+               { 
                          UpdateLocation(yylloc, yyextra, yyleng);
                          yylval->integerVal = atoi(yytext); 
                          return INTEGER_TOKEN; 
                        }

    /* Identifiers */
[A-Z][a-zA-Z0-9_]*     { 
                          UpdateLocation(yylloc, yyextra, yyleng);
                          yylval->literalVal = strdup(yytext); 
                          return CLASS_ID_TOKEN; 
                        }  
[a-z][a-zA-Z0-9_]*     { 
                          UpdateLocation(yylloc, yyextra, yyleng);
                          yylval->literalVal = strdup(yytext); 
                          return OBJECT_ID_TOKEN; 
                        }

    /* White spaces and newlines */
"\n"                   { yyextra->currentLine++; yyextra->currentColumn = 1; }
[ \f\r\t\v]            { yyextra->currentColumn++; }

   /* Strings */
"\""                   { yyextra->stringText.clear(); BEGIN(STRING); }
<STRING>"\""           { 
                            BEGIN(INITIAL); 
                            yylval->literalVal = yyextra->stringText;
                            if (yylval->literalVal.length() > MAX_STRING_LENGTH) { 
                                return SCANNER_ERROR_STRING_EXCEEDS_MAXLENGTH; 
                            } 
                            return STRING_TOKEN; 
                       }
<STRING>"\\\n"         { yyextra->stringText.append("\\\n"); }
<STRING>"\\n"          { yyextra->stringText.push_back('\n'); }
<STRING>"\n"           { 
                           BEGIN(INITIAL); 
                           return SCANNER_ERROR_STRING_CONTAINS_NEWLINE_CHARACTER; 
                       }
<STRING>"\0"           { 
                           BEGIN(INITIAL); 
                           return SCANNER_ERROR_STRING_CONTAINS_NULL_CHARACTER; 
                       }
<STRING>"\\0"          { yyextra->stringText.push_back('\0'); }
<STRING>"\\b"          { yyextra->stringText.push_back('\b'); }
<STRING>"\\t"          { yyextra->stringText.push_back('\t'); }
<STRING>"\\f"          { yyextra->stringText.push_back('\f'); }
<STRING>"\\"[^btf]     { yyextra->stringText.push_back(yytext[1]); }
<STRING>.              { yyextra->stringText.push_back(yytext[0]); }
<STRING><<EOF>>        { 
                           BEGIN(INITIAL); 
                           return SCANNER_ERROR_UNTERMINATED_STRING; 
                       }

   /* End of file */
<<EOF>>                { return 0; }

   /* All other characters are invalid and should trigger an error */
.                      { yyextra->current_column++; return SCANNER_ERROR_INVALID_CHARACTER; } 

%%
