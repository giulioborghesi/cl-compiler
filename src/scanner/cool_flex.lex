%{

#include <cstdlib>
#include <string>

#include <cool/scanner/cool_flex.h>

std::string buffer;

uint32_t lloc = 0;
uint32_t cloc = 0;

uint32_t nestedComment = 0;

void saveLoc();

%}

DIGIT  [0-9]

%option noyywrap

%x STRING INCOMMENT COMMENT

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
"--"                   { cloc += 2; BEGIN(INCOMMENT); }
<INCOMMENT>[\n]        { cloc = 0; lloc += 1; BEGIN(INITIAL); }
<INCOMMENT>.           { ; }
<INCOMMENT><<EOF>>     { BEGIN(INITIAL); return TOKEN_EOF; }

    /* Out-of-line comment */
"(*"                   { saveLoc(); cloc += 2; BEGIN(COMMENT); nestedComment += 1; }
<COMMENT>"(*"          { cloc += 2; nestedComment += 1; }
<COMMENT>"*)"          { cloc += 2; nestedComment -= 1; if (nestedComment == 0) { BEGIN(INITIAL); } }
<COMMENT>[\n]          { cloc = 0; lloc += 1; }
<COMMENT>.             { cloc += 1; }
<COMMENT><<EOF>>       { BEGIN(INITIAL); return -5; }

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

    /* Column, semicolumn and at */
"@"                    { saveLoc(); cloc += 1; return TOKEN_AT; }
":"                    { saveLoc(); cloc += 1; return TOKEN_COLUMN; }     
";"                    { saveLoc(); cloc += 1; return TOKEN_SEMICOLUMN; }

    /* White spaces and newlines */
[\n]                   { cloc = 0; lloc += 1; }
[ \f\r\t\v]            { cloc += 1; }

   /* Strings */
"\""                   { buffer.clear(); BEGIN(STRING); }
<STRING><<EOF>>        { return -4; }
<STRING>[^\\\"\n]*     { buffer.append(yytext); }
<STRING>("\\\n")       { buffer.append(yytext); }
<STRING>"\\"[^bntf]    { buffer.push_back(yytext[1]); }        
<STRING>"\\b"          { buffer.push_back('\b'); }
<STRING>"\\t"          { buffer.push_back('\t'); }
<STRING>"\\f"          { buffer.push_back('\f'); }
<STRING>"\\n"|"\n"     { return -3; }
<STRING>"\""           { yylval.string_val = strdup(buffer.c_str()); BEGIN(INITIAL); return TOKEN_STRING; }

   /* End of file */
<<EOF>>                { return TOKEN_EOF; }

   /* All other characters are invalid and should trigger an error */
.                      { return -2; } 

%%

void saveLoc() {
    yylval.lloc = lloc;
    yylval.cloc = cloc;
}