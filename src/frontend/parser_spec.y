%define api.pure full

%code top {

#include <cool/core/log_message.h>
#include <cool/core/logger_collection.h>
#include <cool/frontend/error_codes.h>
#include <cool/frontend/scanner_extra.h>
#include <cool/ir/class.h>
#include <cool/ir/expr.h>

#include <memory>
#include <vector>

typedef struct cool::ExtraState* YY_EXTRA_TYPE;
typedef void *yyscan_t;

struct YYLTYPE;

/// Helper function to extract the extra argument taken by the lexer
YY_EXTRA_TYPE yyget_extra(yyscan_t);

/// Helper function to install the built-in COOL classes
std::vector<cool::ClassNodePtr> InstallBuiltInClasses(std::vector<cool::ClassNodePtr> classes);

/// Dummy error function prototype -- unused but required by Bison
void yyerror (YYLTYPE*, cool::LoggerCollection*, yyscan_t, cool::ProgramNodePtr*, char const *);

/// Actual error function
void LogError(const cool::FrontEndErrorCode code, const uint32_t lloc, 
            const uint32_t cloc, cool::LoggerCollection* logger);

}

%code requires {

/// Includes
#include <cool/ir/fwd.h> 

#include <cstdlib>
#include <memory>
#include <string> 
#include <vector>

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

/// Forward declaration of logger collection class
namespace cool {
class LoggerCollection;
}

}

%code provides {

/// Lexer function 
extern int yylex(YYSTYPE *, YYLTYPE*, cool::LoggerCollection*, yyscan_t);

/// Lexer signature used inside lexer-generated file
#undef YY_DECL
#define YY_DECL int yylex \
    (YYSTYPE * yylval_param, YYLTYPE * yylloc_param , cool::LoggerCollection* logger, yyscan_t yyscanner)

}

%param { cool::LoggerCollection* logger}
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
%nterm <formalNodes> formall
%nterm <formalNodes> formalc
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
    $1 = InstallBuiltInClasses(std::move($1));
    $$ = cool::ProgramNode::MakeProgramNode(std::move($1)); *program = $$;
  }
| %empty {
        std::vector<cool::ClassNodePtr> classes;
        $$ = cool::ProgramNode::MakeProgramNode(std::move(classes)); *program = $$;
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
        yyget_extra(state)->lastErrorCode = cool::FrontEndErrorCode::PARSER_ERROR_INVALID_CLASS;
        LogError(cool::FrontEndErrorCode::PARSER_ERROR_INVALID_CLASS, 
            @2.first_line, @2.first_column, logger);
        $$ = std::move($1); 
    }
; 

class_: CLASS_TOKEN CLASS_ID_TOKEN '{' features '}' {
        $$ = cool::ClassNode::MakeClassNode(
            $2, "Object", $4, false, @1.first_line, @1.first_column
        );
    }
| CLASS_TOKEN CLASS_ID_TOKEN INHERITS_TOKEN CLASS_ID_TOKEN '{' features '}' {
        $$ = cool::ClassNode::MakeClassNode(
            $2, $4, $6, false, @1.first_line, @1.first_column
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
        yyget_extra(state)->lastErrorCode = cool::FrontEndErrorCode::PARSER_ERROR_INVALID_FEATURE;
        LogError(cool::FrontEndErrorCode::PARSER_ERROR_INVALID_FEATURE,
            @2.first_line, @2.first_column, logger);
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
| OBJECT_ID_TOKEN '(' formalc ')' ':' CLASS_ID_TOKEN '{' expr '}' {
        $$ = cool::MethodNode::MakeMethodNode(
            $1, $6, $3, $8, @1.first_line, @1.first_column
        );
    }
;

/* Methods formal arguments */
formalc : %empty { 
        $$ = std::vector<cool::FormalNodePtr>(); 
    }
| formall {
        $$ = std::move($1);
    }
;

formall : formal {
        $$ = std::vector<cool::FormalNodePtr>{$1};
    }
| formall ',' formal {
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
        yyget_extra(state)->lastErrorCode = cool::FrontEndErrorCode::PARSER_ERROR_INVALID_EXPRESSION;
        LogError(cool::FrontEndErrorCode::PARSER_ERROR_INVALID_EXPRESSION,
            @1.first_line, @1.first_column, logger);
        $$ = std::vector<cool::ExprNodePtr>(); 
    }
| exprs expr ';' { 
        $$ = std::move($1); $$.push_back($2); 
    }
| exprs error ';' { 
        yyget_extra(state)->lastErrorCode = cool::FrontEndErrorCode::PARSER_ERROR_INVALID_EXPRESSION;
        LogError(cool::FrontEndErrorCode::PARSER_ERROR_INVALID_EXPRESSION,
            @2.first_line, @2.first_column, logger);
        $$ = std::move($1); 
    }
;

/* Expressions list, comma separated */
exprsc: '(' ')' {
        $$ = std::vector<cool::ExprNodePtr>();
    }
| '(' exprsl ')' {
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
casebinding: OBJECT_ID_TOKEN ':' CLASS_ID_TOKEN "=>" expr {
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

std::vector<cool::ClassNodePtr> InstallBuiltInClasses(std::vector<cool::ClassNodePtr> classes) {
    std::vector<cool::ClassNodePtr> targetClasses;

    /// Install Object class
    {
        std::vector<cool::FormalNodePtr> emptyArgs;
        std::vector<cool::GenericAttributeNodePtr> attrs;
        attrs.push_back(cool::MethodNode::MakeMethodNode("abort", "Object", emptyArgs, nullptr, 0, 0));
        attrs.push_back(cool::MethodNode::MakeMethodNode("copy", "SELF_TYPE", emptyArgs, nullptr, 0, 0));
        attrs.push_back(cool::MethodNode::MakeMethodNode("type_name", "String", emptyArgs, nullptr, 0, 0));
        targetClasses.push_back(cool::ClassNode::MakeClassNode("Object", "", attrs, true, 0, 0));
    }

    /// Install Int and Bool classes
    {
        std::vector<cool::GenericAttributeNodePtr> emptyAttrs;
        targetClasses.push_back(cool::ClassNode::MakeClassNode("Int", "Object", emptyAttrs, true, 0, 0));
        targetClasses.push_back(cool::ClassNode::MakeClassNode("Bool", "Object", emptyAttrs, true, 0, 0));
    }

    /// Install IO class
    {
        std::vector<cool::FormalNodePtr> args;
        std::vector<cool::GenericAttributeNodePtr> attrs;

        /// Methods with no arguments first
        attrs.push_back(cool::MethodNode::MakeMethodNode("in_string", "String", args, nullptr, 0, 0));
        attrs.push_back(cool::MethodNode::MakeMethodNode("in_int", "Int", args, nullptr, 0, 0));

        /// Remaining methods
        args.push_back(cool::FormalNode::MakeFormalNode("x", "String", 0, 0));
        attrs.push_back(cool::MethodNode::MakeMethodNode("out_string", "SELF_TYPE", args, nullptr, 0, 0));

        args.pop_back();
        args.push_back(cool::FormalNode::MakeFormalNode("x", "Int", 0, 0));
        attrs.push_back(cool::MethodNode::MakeMethodNode("out_int", "SELF_TYPE", args, nullptr, 0, 0));

        /// Install class
        targetClasses.push_back(cool::ClassNode::MakeClassNode("IO", "Object", attrs, true, 0, 0));
    }

    /// Install String class
    {
        std::vector<cool::FormalNodePtr> args;
        std::vector<cool::GenericAttributeNodePtr> attrs;

        /// Method with no arguments first
        attrs.push_back(cool::MethodNode::MakeMethodNode("length", "Int", args, nullptr, 0, 0));

        /// Remaining methods
        args.push_back(cool::FormalNode::MakeFormalNode("s", "String", 0, 0));
        attrs.push_back(cool::MethodNode::MakeMethodNode("concat", "String", args, nullptr, 0, 0));

        args.pop_back();
        args.push_back(cool::FormalNode::MakeFormalNode("i", "Int", 0, 0));
        args.push_back(cool::FormalNode::MakeFormalNode("l", "Int", 0, 0));
        attrs.push_back(cool::MethodNode::MakeMethodNode("substr", "String", args, nullptr, 0, 0));

        /// Install class
        targetClasses.push_back(cool::ClassNode::MakeClassNode("String", "Object", attrs, true, 0, 0));
    }

    /// Copy parsed classes
    for (auto classNode: classes) {
        targetClasses.push_back(classNode);
    }

    return targetClasses;
}

void LogError(const cool::FrontEndErrorCode code, const uint32_t lloc, const uint32_t cloc, cool::LoggerCollection* logger) {
    
    static std::unordered_map<cool::FrontEndErrorCode, std::string> sErrorToString = {
        {cool::FrontEndErrorCode::PARSER_ERROR_INVALID_CLASS, "invalid class definition"},
        {cool::FrontEndErrorCode::PARSER_ERROR_INVALID_FEATURE, "invalid feature definition"},
        {cool::FrontEndErrorCode::PARSER_ERROR_INVALID_EXPRESSION, "invalid expression definition"}
    };

    /// Do nothing if logger is not registered
    if (!logger) {
        return;
    }

    /// Guard against unexpected errors
    assert(sErrorToString.count(code) > 0);

    /// Log error message
    logger->logMessage(cool::LogMessage::MakeErrorMessage("line: %d, col: %d: Error: %s", 
        lloc, cloc, sErrorToString[code]));
}

void yyerror (YYLTYPE* yylloc, cool::LoggerCollection*, yyscan_t state, cool::ProgramNodePtr*, char const *) { }
