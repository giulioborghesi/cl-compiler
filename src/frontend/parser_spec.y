%define api.pure full

%code top {

#include <cool/ir/class.h>
#include <cool/ir/expr.h>

#include <memory>
#include <vector>

}

%code requires {

/// Includes
#include <cool/ir/fwd.h> 

#include <cstdlib>
#include <memory>
#include <string> 
#include <vector>

/// Lexer error codes
#define SCANNER_ERROR_INVALID_CHARACTER -1
#define SCANNER_ERROR_UNTERMINATED_COMMENT -2
#define SCANNER_ERROR_STRING_CONTAINS_NULL_CHARACTER -3
#define SCANNER_ERROR_STRING_CONTAINS_NEWLINE_CHARACTER -4
#define SCANNER_ERROR_UNTERMINATED_STRING -5
#define SCANNER_ERROR_STRING_EXCEEDS_MAXLENGTH -6

/// Parser return type
struct YYSTYPE {
    int32_t integerVal;
    std::string literalVal;
    
    cool::ProgramNodePtr programNode;
    cool::ClassNodePtr classNode;
    cool::GenericAttributeNodePtr attributeNode;

    std::vector<cool::ClassNodePtr> classNodes;
    std::vector<cool::GenericAttributeNodePtr> attributeNodes;
};

/// Lexer state type
typedef void *yyscan_t;

}

%code provides {

extern int yylex(YYSTYPE *, YYLTYPE*, yyscan_t);

/// Error function prototype
void yyerror (YYLTYPE*, yyscan_t, char const *);

}

%param { yyscan_t state }

%define api.value.type {struct YYSTYPE}

/* Nonterminals */
%nterm <programNode> program
%nterm <classNodes> classes
%nterm <classNode> class_
%nterm <attributeNodes> features
%nterm <attributeNode> feature

/* Terminals */
%token <literalVal> CLASS_ID_TOKEN
%token <literalVal> OBJECT_ID_TOKEN
%token <literalVal> STRING_TOKEN 
%token <integerVal> INTEGER_TOKEN

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
%token LESS_EQUAL_TOKEN "<=" 

/* Punctuation */
%token ':'
%token ','
%token '{'
%token '('
%token '}'
%token ')'
%token ';'

/* Operators precedence */
%right "<-"
%precedence NOT_TOKEN
%precedence "<=" '<' '='
%left '+' '-'
%left '*' '/'
%left ISVOID_TOKEN
%nonassoc COMPLEMENT_TOKEN
%nonassoc '@'
%nonassoc '.'

%%

/* Classes and classes features */
program:  classes { $$ = cool::ProgramNode::MakeProgramNode($1); }
;       

classes:  class_ ';' { $$ = std::vector<cool::ClassNodePtr>{$1}; }
|  classes class_ ';' { $$ = std::move($1); $$.push_back($2); }
; 

class_: CLASS_TOKEN CLASS_ID_TOKEN '{' features '}' {
    $$ = cool::ClassNode::MakeClassNode($2, "", $4, @1.first_line, @1.first_column);
  }
| CLASS_TOKEN CLASS_ID_TOKEN INHERITS_TOKEN CLASS_ID_TOKEN '{' features '}' { }
;

features: %empty { $$ = std::vector<cool::GenericAttributeNodePtr>(); }
| features feature ';' { $$ = std::move($1); $$.push_back($2); }
;

feature: OBJECT_ID_TOKEN { $$ = nullptr; }

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

void yyerror (yyscan_t state, char const *) {
    return;
}
