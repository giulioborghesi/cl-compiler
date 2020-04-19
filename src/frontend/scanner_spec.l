%{

#include <cstdlib>
#include <string>

#include <cool/frontend/scanner_spec.h>

/// Maximum buffer length
static constexpr size_t MAX_STRING_LENGTH = 1024;

struct ExtraState {
    uint32_t openComments;
    std::string stringText;
};

%}

DIGIT  [0-9]

%option noyywrap reentrant bison-bridge bison-locations
%option extra-type="ExtraState*"

%x STRING INLINECOMMENT COMMENT

%%

    /* Keywords */
(?i:case)              { return CASE_TOKEN; }
(?i:class)             { return CLASS_TOKEN; }
(?i:else)              { return ELSE_TOKEN; }
f(?i:alse)             { return FALSE_TOKEN; }
(?i:fi)                { return FI_TOKEN; }
(?i:if)                { return IF_TOKEN; }
(?i:in)                { return IN_TOKEN; }
(?i:inherits)          { return INHERITS_TOKEN; }
(?i:isvoid)            { return ISVOID_TOKEN; }
(?i:let)               { return LET_TOKEN; }
(?i:loop)              { return LOOP_TOKEN; }
(?i:new)               { return NEW_TOKEN; }
(?i:not)               { return NOT_TOKEN; }
(?i:pool)              { return POOL_TOKEN; }
(?i:then)              { return THEN_TOKEN; }
t(?i:rue)              { return TRUE_TOKEN; }
(?i:while)             { return WHILE_TOKEN; }
(?i:esac)              { return ESAC_TOKEN; }
(?i:of)                { return OF_TOKEN; }

    /* Assignment operator */
"<-"                   { return ASSIGN_TOKEN; }

    /* Case operator */
"=>"                   { return CASE_OPERATOR_TOKEN; }

    /* In-line comment */
"--"                   { BEGIN(INLINECOMMENT); }
<INLINECOMMENT>[\n]    { BEGIN(INITIAL); }
<INLINECOMMENT>.       { }
<INLINECOMMENT><<EOF>> { BEGIN(INITIAL); return 0; }

    /* Out-of-line comment */
"(\*"                  { BEGIN(COMMENT); yyextra->openComments = 1; }
<COMMENT>"(\*"         { yyextra->openComments += 1; }
<COMMENT>"\*)"         { yyextra->openComments -= 1; if (yyextra == 0) { BEGIN(INITIAL); }}
<COMMENT>[\n]          { }
<COMMENT>.             { }
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
","                    {  return yytext[0]; }

    /* Comparison operators */
"<="                   { return LESS_EQUAL_TOKEN; }

    /* Integers */
{DIGIT}+               { yylval->integerVal = atoi(yytext); return INTEGER_TOKEN; }

    /* Identifiers */
[A-Z][a-zA-Z0-9_]*     { yylval->literalVal = strdup(yytext); return CLASS_ID_TOKEN; }  
[a-z][a-zA-Z0-9_]*     { yylval->literalVal = strdup(yytext); return OBJECT_ID_TOKEN; }

    /* White spaces and newlines */
"\n"                   { }
[ \f\r\t\v]            { }

   /* Strings */
"\""                   { yyextra->stringText.clear() ; BEGIN(STRING); }
<STRING>"\""           { 
                            BEGIN(INITIAL); yylval->literalVal = yyextra->stringText; 
                            if (yylval->literalVal.length() > MAX_STRING_LENGTH) { 
                                return SCANNER_ERROR_STRING_EXCEEDS_MAXLENGTH; 
                            } 
                            return STRING_TOKEN; 
                       }
<STRING>"\\\n"         { yyextra->stringText.push_back('\\'); yyextra->stringText.push_back('\n'); }
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
<STRING><<EOF>>        { 
                           BEGIN(INITIAL); 
                           return SCANNER_ERROR_UNTERMINATED_STRING; 
                       }
<STRING>"\\b"          { yyextra->stringText.push_back('\b'); }
<STRING>"\\t"          { yyextra->stringText.push_back('\t'); }
<STRING>"\\f"          { yyextra->stringText.push_back('\f'); }
<STRING>"\\"[^btf]     { yyextra->stringText.push_back(yytext[1]); }

   /* End of file */
<<EOF>>                { return 0; }

   /* All other characters are invalid and should trigger an error */
.                      { return SCANNER_ERROR_INVALID_CHARACTER; } 

%%
