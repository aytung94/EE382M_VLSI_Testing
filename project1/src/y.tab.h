#define BLIF_MODEL 257
#define BLIF_INPUTS 258
#define BLIF_OUTPUTS 259
#define BLIF_NAMES 260
#define BLIF_END 261
#define NAME 262
#define GATE_COVER 263
#define LINE_CONT 264
#define BLIF_SEQ 265
#define BLIF_FSM 266
#ifdef YYSTYPE
#undef  YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
#endif
#ifndef YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
typedef union {
  int ival;
  char* sval;
} YYSTYPE;
#endif /* !YYSTYPE_IS_DECLARED */
extern YYSTYPE yylval;
