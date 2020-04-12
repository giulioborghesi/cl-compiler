#ifndef COOL_SCANNER_COOL_FLEX_H
#define COOL_SCANNER_COOL_FLEX_H

#include <cstdlib>

/// Token definition
#define TOKEN_CLASS 258
#define TOKEN_ELSE 259
#define TOKEN_FALSE 260
#define TOKEN_FI 261
#define TOKEN_IF 262
#define TOKEN_IN 263
#define TOKEN_INHERITS 264
#define TOKEN_ISVOID 265
#define TOKEN_LET 266
#define TOKEN_LOOP 267
#define TOKEN_POOL 268
#define TOKEN_THEN 269
#define TOKEN_WHILE 270
#define TOKEN_CASE 271
#define TOKEN_ESAC 272
#define TOKEN_NEW 273
#define TOKEN_OF 274
#define TOKEN_NOT 275
#define TOKEN_TRUE 276
#define TOKEN_ASSIGN 277
#define TOKEN_PLUS 278
#define TOKEN_MINUS 279
#define TOKEN_MULT 280
#define TOKEN_DIVIDE 281
#define TOKEN_LESS_EQUAL 282
#define TOKEN_LESS 283
#define TOKEN_EQUAL 284
#define TOKEN_COMPLEMENT 285
#define TOKEN_LEFT_PARENTHESIS 286
#define TOKEN_RIGHT_PARENTHESIS 287
#define TOKEN_LEFT_CURLY_BRACE 288
#define TOKEN_RIGHT_CURLY_BRACE 289
#define TOKEN_INT 290
#define TOKEN_CLASS_ID 291
#define TOKEN_OBJECT_ID 292
#define TOKEN_AT 293
#define TOKEN_DOT 294
#define TOKEN_COLUMN 295
#define TOKEN_SEMICOLUMN 296
#define TOKEN_STRING 297
#define TOKEN_EOF 298

#if !defined YYSTYPE && !defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE {
  int int_val;
  char *string_val;
  uint32_t lloc;
  uint32_t cloc;
} YYSTYPE;

#define yystype YYSTYPE
#define YYSTYPE_IS_DECLARED 1
#define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

#endif
