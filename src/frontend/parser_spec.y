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

    cool::CaseBindingNodePtr caseBinding;
    cool::ClassNodePtr classNode;
    cool::ExprNodePtr exprNode;
    cool::FormalNodePtr formalNode;
    cool::LetBindingNodePtr letBinding;
    cool::GenericAttributeNodePtr attributeNode;
    cool::ProgramNodePtr programNode;

    std::vector<cool::CaseBindingNodePtr> caseBindings;
    std::vector<cool::ClassNodePtr> classNodes;
    std::vector<cool::FormalNodePtr> formalNodes;
    std::vector<cool::GenericAttributeNodePtr> attributeNodes;
    std::vector<cool::ExprNodePtr> exprNodes;
    std::vector<cool::LetBindingNodePtr> letBindings;
};

/// Lexer state type
typedef void *yyscan_t;

}

%code provides {

extern int yylex(YYSTYPE *, YYLTYPE*, yyscan_t);

/// Error function prototype
void yyerror (YYLTYPE*, yyscan_t, cool::ProgramNodePtr*, char const *);

}

%param { yyscan_t state }
%parse-param {cool::ProgramNodePtr* program}

%define api.value.type {struct YYSTYPE}

/* Nonterminals */
%nterm <attributeNode> feature
%nterm <attributeNodes> features
%nterm <caseBinding> casebinding
%nterm <caseBindings> casebindings
%nterm <classNode> class_
%nterm <classNodes> classes
%nterm <exprNode> expr
%nterm <exprNodes> exprs
%nterm <exprNodes> exprsc
%nterm <exprNodes> exprsl
%nterm <formalNode> formal
%nterm <formalNodes> formals
%nterm <letBinding> letbinding
%nterm <letBindings> letbindings 
%nterm <programNode> program

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
%token CASE_OPERATOR_TOKEN "=>"
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
%right IN_TOKEN
%right "<-"
%left NOT_TOKEN
%nonassoc "<=" '<' '='
%left '+' '-'
%left '*' '/'
%left ISVOID_TOKEN
%nonassoc '~'
%nonassoc '@'
%nonassoc '.'

%%

/* Classes */
program:  classes { 
    $$ = cool::ProgramNode::MakeProgramNode($1); *program = $$;
  }
;       

classes:  class_ ';' { 
    $$ = std::vector<cool::ClassNodePtr>{$1}; 
  }
| error ';' {
    $$ = std::vector<cool::ClassNodePtr>();
  }
| classes class_ ';' { 
    $$ = std::move($1); $$.push_back($2); 
  }
| classes error ';' {
    $$ = std::move($1); 
  }
; 

class_: CLASS_TOKEN CLASS_ID_TOKEN '{' features '}' {
    $$ = cool::ClassNode::MakeClassNode(
      $2, "", $4, @1.first_line, @1.first_column
    );
  }
| CLASS_TOKEN CLASS_ID_TOKEN INHERITS_TOKEN CLASS_ID_TOKEN '{' features '}' {
    $$ = cool::ClassNode::MakeClassNode(
      $2, $4, $6, @1.first_line, @1.first_column
    ); 
  }
;

/* Class features */
features: %empty { 
    $$ = std::vector<cool::GenericAttributeNodePtr>(); 
  }
| features feature ';' { 
    $$ = std::move($1); $$.push_back($2); 
  }
| features error ';' {
    $$ = std::move($1);
  }
;

feature: OBJECT_ID_TOKEN ':' CLASS_ID_TOKEN { 
    $$ = cool::AttributeNode::MakeAttributeNode(
      $1, $3, nullptr, @1.first_line, @1.first_column
    );
  }
| OBJECT_ID_TOKEN ':' CLASS_ID_TOKEN "<-" expr {
    $$ = cool::AttributeNode::MakeAttributeNode(
      $1, $3, $5, @1.first_line, @1.first_column
    );
  }
| OBJECT_ID_TOKEN '(' formals ')' ':' CLASS_ID_TOKEN '{' expr '}' {
    $$ = cool::MethodNode::MakeMethodNode(
      $1, $6, $3, @1.first_line, @1.first_column
    );
  }
;

/* Methods formal arguments */
formals : formal {
    $$ = std::vector<cool::FormalNodePtr>{$1};
  }
| formals ',' formal {
    $$ = std::move($1); $$.push_back($3);
  }
;

formal : OBJECT_ID_TOKEN ':' CLASS_ID_TOKEN {
    $$ = cool::FormalNode::MakeFormalNode(
      $1, $3, @1.first_line, @1.first_column
    );
  }
;

/* Expressions list, semi-column separated */
exprs: expr ';' { 
    $$ = std::vector<cool::ExprNodePtr>{$1}; 
  }
| error ';' { 
    $$ = std::vector<cool::ExprNodePtr>(); 
  }
| exprs expr ';' { 
    $$ = std::move($1); $$.push_back($2); 
  }
| exprs error ';' { 
    $$ = std::move($1); 
  }
;

/* Expressions list, comma separated */
exprsc: '(' exprsl ')' {
    $$ = std::move($2);
  }
;

exprsl: expr {
    $$ = std::vector<cool::ExprNodePtr>{$1};
  }
| exprsl ',' expr {
    $$ = std::move($1); $$.push_back($3);
  }
;

/* Expressions */
expr: OBJECT_ID_TOKEN "<-" expr { 
    $$ = cool::AssignmentExprNode::MakeAssignmentExprNode(
      $1, $3, @1.first_line, @1.first_column
    ); 
  }
| expr '@' CLASS_ID_TOKEN '.' OBJECT_ID_TOKEN exprsc {
    $$ = cool::StaticDispatchExprNode::MakeStaticDispatchExprNode(
      $5, $3, $1, $6, @1.first_line, @1.first_column
    );
  }
| expr '.' OBJECT_ID_TOKEN exprsc {
    $$ = cool::DispatchExprNode::MakeDispatchExprNode(
      $3, $1, $4, @1.first_line, @1.first_column
    );
  }
| OBJECT_ID_TOKEN exprsc {
    $$ = cool::DispatchExprNode::MakeDispatchExprNode(
      $1, nullptr, $2, @1.first_line, @1.first_column
    );
  }
| CASE_TOKEN expr OF_TOKEN casebindings ESAC_TOKEN {
    $$ = cool::CaseExprNode::MakeCaseExprNode(
      $4, $2, @1.first_line, @1.first_column
    );
  }
| IF_TOKEN expr THEN_TOKEN expr ELSE_TOKEN expr FI_TOKEN {
    $$ = cool::IfExprNode::MakeIfExprNode(
      $2, $4, $6, @1.first_line, @1.first_column
    );
  }
| ISVOID_TOKEN expr { 
    $$ = cool::UnaryExprNode::MakeUnaryExprNode(
      $2, cool::UnaryOpID::IsVoid, @1.first_line, @1.first_column
    ); 
  }
| LET_TOKEN letbindings IN_TOKEN expr {
    $$ = cool::LetExprNode::MakeLetExprNode(
      std::move($2), $4, @1.first_line, @1.first_column
    );
}
| NEW_TOKEN CLASS_ID_TOKEN { 
    $$ = cool::NewExprNode::MakeNewExprNode(
      $2, @1.first_line, @1.first_column
    ); 
  }
| NOT_TOKEN expr {
    $$ = cool::UnaryExprNode::MakeUnaryExprNode(
      $2, cool::UnaryOpID::Not, @1.first_line, @1.first_column
    ); 
  }
| OBJECT_ID_TOKEN {
    $$ = cool::IdExprNode::MakeIdExprNode(
      $1, @1.first_line, @1.first_column
    );
  }
| WHILE_TOKEN expr LOOP_TOKEN expr POOL_TOKEN {
    $$ = cool::WhileExprNode::MakeWhileExprNode(
      $2, $4, @1.first_line, @1.first_column
    );
  }
| '{' exprs '}' { 
    $$ = cool::BlockExprNode::MakeBlockExprNode(
      std::move($2), @1.first_line, @1.first_column
    ); 
  }
| '(' expr ')' { 
    $$ = $2; 
  }
| expr '+' expr { 
    $$ = cool::BinaryExprNode<cool::ArithmeticOpID>::MakeBinaryExprNode(
      $1, $3, cool::ArithmeticOpID::Plus, @1.first_line, @1.first_column
    );
  }
| expr '-' expr { 
    $$ = cool::BinaryExprNode<cool::ArithmeticOpID>::MakeBinaryExprNode(
      $1, $3, cool::ArithmeticOpID::Minus, @1.first_line, @1.first_column
    );
  }
| expr '*' expr { 
    $$ = cool::BinaryExprNode<cool::ArithmeticOpID>::MakeBinaryExprNode(
      $1, $3, cool::ArithmeticOpID::Mult, @1.first_line, @1.first_column
    );
  }
| expr '/' expr { 
    $$ = cool::BinaryExprNode<cool::ArithmeticOpID>::MakeBinaryExprNode(
      $1, $3, cool::ArithmeticOpID::Div, @1.first_line, @1.first_column
    );
  }
| expr '<' expr { 
    $$ = cool::BinaryExprNode<cool::ComparisonOpID>::MakeBinaryExprNode(
      $1, $3, cool::ComparisonOpID::LessThan, @1.first_line, @1.first_column
    );
  }
| expr "<=" expr { 
    $$ = cool::BinaryExprNode<cool::ComparisonOpID>::MakeBinaryExprNode(
      $1, $3, cool::ComparisonOpID::LessThanOrEqual, @1.first_line, @1.first_column
    );
  }
| expr '=' expr { 
    $$ = cool::BinaryExprNode<cool::ComparisonOpID>::MakeBinaryExprNode(
      $1, $3, cool::ComparisonOpID::Equal, @1.first_line, @1.first_column
    );
  }
| '~' expr { 
    $$ = cool::UnaryExprNode::MakeUnaryExprNode(
      $2, cool::UnaryOpID::Complement, @1.first_line, @1.first_column
    ); 
  }
| FALSE_TOKEN {
    $$ = cool::BooleanExprNode::MakeBooleanExprNode(
      false, @1.first_line, @1.first_column
    );
  }
| INTEGER_TOKEN {
    $$ = cool::LiteralExprNode<int32_t>::MakeLiteralExprNode(
      $1, @1.first_line, @1.first_column
    );
  }
| STRING_TOKEN {
    $$ = cool::LiteralExprNode<std::string>::MakeLiteralExprNode(
      $1, @1.first_line, @1.first_column
    );
  }
| TRUE_TOKEN {
    $$ = cool::BooleanExprNode::MakeBooleanExprNode(
      true, @1.first_line, @1.first_column
    );
  }
;

/* Case bindings list */
casebindings: casebinding ';' {
    $$ = std::vector<cool::CaseBindingNodePtr>{$1};
  }
| casebindings casebinding ';' {
    $$ = std::move($1); $$.push_back($2);
  }
;

/* Case bindings */
casebinding: OBJECT_ID_TOKEN ':' CLASS_ID_TOKEN "=>" expr ';' {
    $$ = cool::CaseBindingNode::MakeCaseBindingNode(
      $1, $3, $5, @1.first_line, @1.first_column
    );
  }
;

/* Let bindings list */
letbindings: letbinding {
    $$ = std::vector<cool::LetBindingNodePtr>{$1};
  }
| letbindings ',' letbinding {
    $$ = std::move($1); $$.push_back($3);
  }
;

/* Let bindings */
letbinding: OBJECT_ID_TOKEN ':' CLASS_ID_TOKEN {
    $$ = cool::LetBindingNode::MakeLetBindingNode(
      $1, $3, nullptr, @1.first_line, @1.first_column
    );
  }
| OBJECT_ID_TOKEN ':' CLASS_ID_TOKEN "<-" expr {
    $$ = cool::LetBindingNode::MakeLetBindingNode(
      $1, $3, $5, @1.first_line, @1.first_column
    );
  }
;

%%

void yyerror (YYLTYPE* yylloc, yyscan_t state, cool::ProgramNodePtr*, char const *) {
    return;
}
