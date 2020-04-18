%code top {

#include <cool/frontend/parse_result.h>
#include <cool/frontend/scanner.h>
#include <cool/ir/class.h>
#include <cool/ir/expr.h>

#include <memory>
#include <vector>

/// Redefine yylex
#undef yylex
#define yylex scanner->nextToken().getTokenID

/// Define yyerror
void yyerror (cool::Scanner * scanner, char const *);

}

%code requires {

/// Forward declarations
namespace cool {
class Scanner;
struct ParseResult;
}

/// EOF and error codes
#define EOF_TOKEN 0
#define SCANNER_ERROR_INVALID_CHARACTER -1
#define SCANNER_ERROR_UNTERMINATED_COMMENT -2
#define SCANNER_ERROR_STRING_CONTAINS_NULL_CHARACTER -3
#define SCANNER_ERROR_STRING_CONTAINS_NEWLINE_CHARACTER -4
#define SCANNER_ERROR_UNTERMINATED_STRING -5
#define SCANNER_ERROR_STRING_EXCEEDS_MAXLENGTH -6

}

%parse-param {cool::Scanner *scanner}

%define api.value.type {struct cool::ParseResult}

/* Nonterminals */
%nterm <programNode> program
%nterm <classNodes> classes
%nterm <classNode> class

/* Terminals */
%token <literalVal> CLASS_ID_TOKEN
%token <literalVal> INTEGER_TOKEN
%token <literalVal> OBJECT_ID_TOKEN
%token <literalVal> STRING_TOKEN 

/* Keywords */
%token CASE_TOKEN
%token CLASS_TOKEN
%token ELSE_TOKEN
%token ESAC_TOKEN
%token FALSE_TOKEN
%token FI_TOKEN
%token IF_TOKEN
%token IN_TOKEN
%token INHERITS_TOKEN
%token ISVOID_TOKEN
%token LET_TOKEN
%token LOOP_TOKEN
%token NEW_TOKEN
%token NOT_TOKEN
%token OF_TOKEN
%token POOL_TOKEN
%token THEN_TOKEN
%token TRUE_TOKEN
%token WHILE_TOKEN

/* Operators aliases */
%token ASSIGN_TOKEN "<-"
%token EQUAL_TOKEN "="
%token LESS_TOKEN "<"
%token LESS_EQUAL_TOKEN "<=" 
%token PLUS_TOKEN "+"
%token MINUS_TOKEN "-"
%token MULT_TOKEN "*"
%token DIVIDE_TOKEN "/"
%token COMPLEMENT_TOKEN "~"
%token AT_TOKEN "@"
%token DOT_TOKEN "."

/* Punctuation */
%token COLUMN_TOKEN
%token COMMA_TOKEN
%token LEFT_CURLY_BRACE_TOKEN
%token LEFT_PARENTHESIS_TOKEN
%token RIGHT_CURLY_BRACE_TOKEN
%token RIGHT_PARENTHESIS_TOKEN
%token SEMICOLUMN_TOKEN

/* Operators precedence */
%right ASSIGN_TOKEN
%precedence NOT_KEYWORD
%precedence LESS_EQUAL_TOKEN LESS_TOKEN EQUAL_TOKEN
%left PLUS_TOKEN MINUS_TOKEN
%left MULT_TOKEN DIVIDE_TOKEN
%left ISVOID_KEYWORD
%precedence COMPLEMENT_TOKEN
%precedence AT_TOKEN
%precedence DOT_TOKEN

%%

/* Classes and classes features */
program:  classes { $$ = std::shared_ptr<cool::ProgramNode>(cool::ProgramNode::MakeProgramNode(&$1)); }
;       

classes:  class ';' { $$ = std::vector<std::shared_ptr<cool::ClassNode>>(1, $1); }
|  classes class ';' { $1.push_back($2); $$ = std::move($1); }
; 

class: CLASS_TOKEN CLASS_ID_TOKEN '{' features ';' '}' { $$ = std::shared_ptr<cool::ClassNode>(cool::ClassNode::MakeClassNode($2, "", nullptr, nullptr, 0, 0)); }
| CLASS_TOKEN CLASS_ID_TOKEN INHERITS_TOKEN CLASS_ID_TOKEN '{' features '}' { $$ = std::shared_ptr<cool::ClassNode>(cool::ClassNode::MakeClassNode($2, $4, nullptr, nullptr, 0, 0)); }
;

features: %empty
//| features feature ';'
;

/*
feature: OBJECT_ID ':' CLASS_ID
| OBJECT_ID ':' CLASS_ID "<-" expr
| OBJECT_ID '(' ')' ':' CLASS_ID '{' expr '}'
| OBJECT_ID '(' formals ')' ':' CLASS_ID '{' expr '}'
;

formals: formal
| formals ',' formal
;

formal: OBJECT_ID ':' CLASS_ID
;
*/

/* Expressions */
/*
exprs: expr ';'
| exprs expr ';'
;

expr: OBJECT_ID "<-" expr
| OBJECT_ID '(' ')'
| IF_KEYWORD expr THEN_KEYWORD expr ELSE_KEYWORD expr FI_KEYWORD
| WHILE_KEYWORD expr LOOP_KEYWORD expr POOL_KEYWORD
| '{' exprs '}'
| NEW_KEYWORD CLASS_ID
| ISVOID_KEYWORD expr
| expr '+' expr
| expr '-' expr
| expr '*' expr
| expr '/' expr
| '~' expr
| expr '<' expr
| expr "<=" expr
| expr '=' expr
| NOT_KEYWORD expr
| '(' expr ')'
| OBJECT_ID
| INTEGER_VAL
| STRING_VAL
| TRUE_KEYWORD
| FALSE_KEYWORD
;
*/

%%

void yyerror (cool::Scanner * scanner, char const *) {
    return;
}
