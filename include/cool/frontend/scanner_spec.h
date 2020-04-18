#ifndef COOL_FRONTEND_SCANNER_SPEC_H
#define COOL_FRONTEND_SCANNER_SPEC_H

#include <cool/frontend/parse_result.h>

#include <cstdlib>

/// Errors
#define SCANNER_ERROR_INVALID_CHARACTER -1
#define SCANNER_ERROR_UNTERMINATED_COMMENT -2
#define SCANNER_ERROR_STRING_CONTAINS_NULL_CHARACTER -3
#define SCANNER_ERROR_STRING_CONTAINS_NEWLINE_CHARACTER -4
#define SCANNER_ERROR_UNTERMINATED_STRING -5
#define SCANNER_ERROR_STRING_EXCEEDS_MAXLENGTH -6

/// Token definition
#define CLASS_TOKEN 258
#define ELSE_TOKEN 259
#define FALSE_TOKEN 260
#define FI_TOKEN 261
#define IF_TOKEN 262
#define IN_TOKEN 263
#define INHERITS_TOKEN 264
#define ISVOID_TOKEN 265
#define LET_TOKEN 266
#define LOOP_TOKEN 267
#define POOL_TOKEN 268
#define THEN_TOKEN 269
#define WHILE_TOKEN 270
#define CASE_TOKEN 271
#define ESAC_TOKEN 272
#define NEW_TOKEN 273
#define OF_TOKEN 274
#define NOT_TOKEN 275
#define TRUE_TOKEN 276
#define ASSIGN_TOKEN 277
#define LESS_EQUAL_TOKEN 282
#define INTEGER_TOKEN 290
#define CLASS_ID_TOKEN 291
#define OBJECT_ID_TOKEN 292
#define STRING_TOKEN 298
#define EOF_TOKEN 299

// Maximum string length in COOL
#define MAX_STRING_LENGTH 1000

// Define YYSTYPE
#if !defined YYSTYPE && !defined YYSTYPE_IS_DECLARED
typedef struct cool::ParseResult YYSTYPE;
#define yystype YYSTYPE
#define YYSTYPE_IS_DECLARED 1
#define YYSTYPE_IS_TRIVIAL 1
#endif

// Scanner state type alias
typedef void *yyscan_t;

/// YACC buffer state type
typedef struct yy_buffer_state *YY_BUFFER_STATE;

/// YACC lexer state functions
int yylex_init(yyscan_t *);
int yylex_destroy(yyscan_t);

/// YACC buffer creation functions
YY_BUFFER_STATE yy_create_buffer(FILE *, int, yyscan_t);
YY_BUFFER_STATE yy_scan_string(const char *, yyscan_t);

/// YACC buffer destruction functions
void yy_switch_to_buffer(YY_BUFFER_STATE, yyscan_t);
void yy_delete_buffer(YY_BUFFER_STATE, yyscan_t);

/// YACC lexer function
int yylex(YYSTYPE *yylval, yyscan_t yystate);

#endif
