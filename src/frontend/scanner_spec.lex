%{

#include <cstdlib>
#include <string>

#include <cool/frontend/scanner_spec.h>

std::string buffer;

uint32_t lloc = 0;
uint32_t cloc = 0;

uint32_t nestedComment = 0;

void saveLoc();

%}

DIGIT  [0-9]

%option noyywrap

%x STRING INLINECOMMENT COMMENT

%%

    /* Keywords */
(?i:class)             { saveLoc(); cloc += 5; return TOKEN_CLASS; }
(?i:else)              { saveLoc(); cloc += 4; return TOKEN_ELSE; }
f(?i:alse)             { saveLoc(); cloc += 5; return TOKEN_FALSE; }
(?i:fi)                { saveLoc(); cloc += 2; return TOKEN_FI; }
(?i:if)                { saveLoc(); cloc += 2; return TOKEN_IF; }
(?i:in)                { saveLoc(); cloc += 2; return TOKEN_IN; }
(?i:inherits)          { saveLoc(); cloc += 8; return TOKEN_INHERITS; }
(?i:isvoid)            { saveLoc(); cloc += 6; return TOKEN_ISVOID; }
(?i:let)               { saveLoc(); cloc += 3; return TOKEN_LET; }
(?i:loop)              { saveLoc(); cloc += 4; return TOKEN_LOOP; }
(?i:pool)              { saveLoc(); cloc += 4; return TOKEN_POOL; }
(?i:then)              { saveLoc(); cloc += 4; return TOKEN_THEN; }
(?i:while)             { saveLoc(); cloc += 5; return TOKEN_WHILE; }
(?i:case)              { saveLoc(); cloc += 4; return TOKEN_CASE; }
(?i:esac)              { saveLoc(); cloc += 4; return TOKEN_ESAC; }
(?i:new)               { saveLoc(); cloc += 3; return TOKEN_NEW; }
(?i:of)                { saveLoc(); cloc += 2; return TOKEN_OF; }
(?i:not)               { saveLoc(); cloc += 3; return TOKEN_NOT; }
t(?i:rue)              { saveLoc(); cloc += 4; return TOKEN_TRUE; }

    /* Assignment operator */
"<-"                   { saveLoc(); cloc += 2; return TOKEN_ASSIGN; }

    /* In-line comment */
"--"                   { cloc += 2; BEGIN(INLINECOMMENT); }
<INLINECOMMENT>[\n]    { cloc = 0; lloc += 1; BEGIN(INITIAL); }
<INLINECOMMENT>.       { ; }
<INLINECOMMENT><<EOF>> { BEGIN(INITIAL); return TOKEN_EOF; }

    /* Out-of-line comment */
"(\*"                  { saveLoc(); cloc += 2; BEGIN(COMMENT); nestedComment += 1; }
<COMMENT>"(\*"         { cloc += 2; nestedComment += 1; }
<COMMENT>"\*)"         { cloc += 2; nestedComment -= 1; if (nestedComment == 0) { BEGIN(INITIAL); } }
<COMMENT>[\n]          { cloc = 0; lloc += 1; }
<COMMENT>.             { cloc += 1; }
<COMMENT><<EOF>>       { BEGIN(INITIAL); return SCANNER_ERROR_UNTERMINATED_COMMENT; }

    /* Arithmetic operators */
"+"                    { saveLoc(); cloc += 1; return TOKEN_PLUS; }
"-"                    { saveLoc(); cloc += 1; return TOKEN_MINUS; }
"*"                    { saveLoc(); cloc += 1; return TOKEN_MULT; }
"/"                    { saveLoc(); cloc += 1; return TOKEN_DIVIDE; }

    /* Comparison operators */
"<="                   { saveLoc(); cloc += 2; return TOKEN_LESS_EQUAL; }
"<"                    { saveLoc(); cloc += 1; return TOKEN_LESS; }
"="                    { saveLoc(); cloc += 1; return TOKEN_EQUAL; }

    /* Complement operator */
"~"                    { saveLoc(); cloc += 1; return TOKEN_COMPLEMENT; }

    /* Curly brackets and parenthesis */
"("                    { saveLoc(); cloc += 1; return TOKEN_LEFT_PARENTHESIS; }
")"                    { saveLoc(); cloc += 1; return TOKEN_RIGHT_PARENTHESIS; }
"{"                    { saveLoc(); cloc += 1; return TOKEN_LEFT_CURLY_BRACE; }
"}"                    { saveLoc(); cloc += 1; return TOKEN_RIGHT_CURLY_BRACE; }

    /* Integers */
{DIGIT}+               { saveLoc(); cloc += strlen(yytext); yylval.int_val = atoi(yytext); return TOKEN_INT; }

    /* Identifiers */
[A-Z][a-zA-Z0-9_]*     { saveLoc(); cloc += strlen(yytext); yylval.string_val = strdup(yytext); return TOKEN_CLASS_ID; }  
[a-z][a-zA-Z0-9_]*     { saveLoc(); cloc += strlen(yytext); yylval.string_val = strdup(yytext); return TOKEN_OBJECT_ID; }

    /* At, dot, column, semicolumn and comma */
"@"                    { saveLoc(); cloc += 1; return TOKEN_AT; }
"."                    { saveLoc(); cloc += 1; return TOKEN_DOT;}
":"                    { saveLoc(); cloc += 1; return TOKEN_COLUMN; }     
";"                    { saveLoc(); cloc += 1; return TOKEN_SEMICOLUMN; }
","                    { saveLoc(); cloc += 1; return TOKEN_COMMA; }

    /* White spaces and newlines */
"\n"                   { cloc = 0; lloc += 1; }
[ \f\r\t\v]            { cloc += 1; }

   /* Strings */
"\""                   { saveLoc(); buffer.clear(); BEGIN(STRING); }
<STRING>"\""           { cloc += 1; yylval.string_val = strdup(buffer.c_str()); if (buffer.length() > MAX_STRING_LENGTH) { return SCANNER_ERROR_STRING_EXCEEDS_MAXLENGTH; }; return TOKEN_STRING; }
<STRING>"\\\n"         { cloc = 0; lloc += 1; buffer.push_back('\\'); buffer.push_back('\n'); }
<STRING>"\\n"          { cloc += 2; buffer.push_back('\n'); }
<STRING>"\n"           { saveLoc(); return SCANNER_ERROR_STRING_CONTAINS_NEWLINE_CHARACTER; }
<STRING>"\0"           { saveLoc(); return SCANNER_ERROR_STRING_CONTAINS_NULL_CHARACTER; }
<STRING>"\\0"          { cloc += 2; buffer.push_back('\0'); }
<STRING><<EOF>>        { BEGIN(INITIAL); return SCANNER_ERROR_UNTERMINATED_STRING; }
<STRING>"\\b"          { cloc += 2; buffer.push_back('\b'); }
<STRING>"\\t"          { cloc += 2; buffer.push_back('\t'); }
<STRING>"\\f"          { cloc += 2; buffer.push_back('\f'); }
<STRING>"\\"[^btf]     { cloc += 2; buffer.push_back(yytext[1]); }

   /* End of file */
<<EOF>>                { return TOKEN_EOF; }

   /* All other characters are invalid and should trigger an error */
.                      { return SCANNER_ERROR_INVALID_CHARACTER; } 

%%

void saveLoc() {
    yylval.lloc = lloc;
    yylval.cloc = cloc;
}