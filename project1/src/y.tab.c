/* original parser id follows */
/* yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93" */
/* (use YYMAJOR/YYMINOR for ifdefs dependent on parser version) */

#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define YYPATCH 20140715

#define YYEMPTY        (-1)
#define yyclearin      (yychar = YYEMPTY)
#define yyerrok        (yyerrflag = 0)
#define YYRECOVERING() (yyerrflag != 0)
#define YYENOMEM       (-2)
#define YYEOF          0
#define YYPREFIX "yy"

#define YYPURE 0

#line 2 "blif.y"
/******************************************************************\ 
 *
 *
 * This is free software, licensed under the GNU Public License V2.
 * See the file COPYING for details.
 *
 * This code is provided as is, with no warranty implied or expressed.
 * Use at your own risk.  This software may contain bugs which could
 * cause it to malfunction in possibly dangerous ways.
 *  
 * Neither the author, nor any affiliates of the author are responsible for 
 * any damages caused by use of this software.
 *
 * Copyright:  Narayanan Krishnamurthy, Heling Yi, 1997
 *
\******************************************************************/


/******************************************************************\ 
 *
 * This code was developed as part of a project in the digital system 
 * simulation class (Spring 1997 ) taught by 
 * Dr. Nur Touba at The University of Texas at Austin.
 * The project is Path Delay Fault simulation. This file is a "yacc" 
 * file that describes the grammar of a BLIF file that has 
 * been mapped to a technology library containing only 2-input or less
 * combinational gates. The parser parses the BLIF files and calls
 * the Add_Gate() and Build_Ckt() routines in "parse_ckt.c"
\******************************************************************/

#include <stdio.h>
#include <string.h> 
#include "read_ckt.h"

/******************************************************************\
 * STAND_ALONE has to be defined if the parser has to be
 * run in a stand_alone mode with its own "main()" function
 * Otherwise, the function ParseAndBuildCkt() function
 * is called from the read_circuit() function in read_circuit.c
\******************************************************************/

#ifndef STAND_ALONE
int ParseAndBuildCkt(FILE *);
#endif

/******************************************************************\
 * Add_Gate() and Build_Ckt() are functions being called from
 * this yacc file. The functions are defined in "read_ckt.cc"
\******************************************************************/
/*extern  void Add_Gate(Gate_Info_t *);*/
/*extern void Build_Ckt(void);*/

/******************************************************************\
 * DEBUG_MESSAGES  has to be defined to generate debug messages
 * as the BLIF file is being parsed
\******************************************************************/
#ifdef DEBUG_MESSAGES
void PrintGateInfo(void);
#endif


extern char *nameBuf;
extern char *funcCover;

/******************************************************************\
 * NETLIST_DUMP has to be defined to generate the "Output_Netlist"
 * which is a parsed output of the input BLIF file
\******************************************************************/
#ifdef NETLIST_DUMP
FILE *GateFile;
void DumpGateToFile(FILE *, Gate_Info_t *);
#endif

Gate_Info_t		gateInfo; 

/******************************************************************\
 * The following variables and data_structures are defined for 
 * local use within the yacc file. Added an additional UNKNOWN
 * gate because while parsing the signal list of the gate the
 * the gate type is unknown until the cover of the function
 * is read.

 * Currently handles 2-ip AND, OR, NAND, NOR and 1-i/p INV,
 * 1-i/p BUF, constant 0 (GND) and constant 1 (VCC) nodes.
 * PI's and PO's are also handled as separate nodes.
\******************************************************************/
#define MAX_GATE_TYPES 11 

/******************************************************************\
 * Commented this out because Dr. Touba said that it was not necesary
 * consider a generic CONSTANT node but just a PO constant node.
struct Gate_Type_t {
	char *name;
	char *cover;
	int  total;
} gate_type[MAX_GATE_TYPES] = {
                        "AND",  "11 1", 0,
		      			"OR",   "1- 1", 0,
				       	"NAND", "0- 1", 0,
				        "NOR", 	"00 1", 0,
				      	"INV",  "0 1", 0,
				       	"BUF",  "1 1", 0,
				      	"CONSTANT_1", "1", 0,
				      	"CONSTANT_0", "", 0,
				      	"PI", 	"", 0,
				       	"PO", 	"", 0,
			                "UNKNOWN", "", 0
			    };
\******************************************************************/

struct Gate_Type_t {
	char *name;
	char *cover;
	int  total;
} gate_type[MAX_GATE_TYPES] = {
                        "AND",  "11 1", 0,
		      			"OR",   "1- 1", 0,
				       	"NAND", "0- 1", 0,
				        "NOR", 	"00 1", 0,
				      	"INV",  "0 1", 0,
				       	"BUF",  "1 1", 0,
				      	"PI", 	"", 0,
				       	"PO", 	"", 0,
				      	"PO_GND", "", 0,
				      	"PO_VCC", "1", 0,
			                "UNKNOWN", "", 0
			    };
Gate_Type_Enum		nodeKind = UNKNOWN;   
int			signalNum; 
char                    **signalArray;

#line 135 "blif.y"
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
#line 165 "y.tab.c"

/* compatibility with bison */
#ifdef YYPARSE_PARAM
/* compatibility with FreeBSD */
# ifdef YYPARSE_PARAM_TYPE
#  define YYPARSE_DECL() yyparse(YYPARSE_PARAM_TYPE YYPARSE_PARAM)
# else
#  define YYPARSE_DECL() yyparse(void *YYPARSE_PARAM)
# endif
#else
# define YYPARSE_DECL() yyparse(void)
#endif

/* Parameters sent to lex. */
#ifdef YYLEX_PARAM
# define YYLEX_DECL() yylex(void *YYLEX_PARAM)
# define YYLEX yylex(YYLEX_PARAM)
#else
# define YYLEX_DECL() yylex(void)
# define YYLEX yylex()
#endif

/* Parameters sent to yyerror. */
#ifndef YYERROR_DECL
#define YYERROR_DECL() yyerror(const char *s)
#endif
#ifndef YYERROR_CALL
#define YYERROR_CALL(msg) yyerror(msg)
#endif

extern int YYPARSE_DECL();

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
#define YYERRCODE 256
typedef short YYINT;
static const YYINT yylhs[] = {                           -1,
    0,    0,    0,    3,    4,    4,    4,    4,    5,    5,
    5,    5,    1,    1,    1,    6,    6,    8,    9,    2,
    2,    2,    7,    7,
};
static const YYINT yylen[] = {                            2,
    5,    5,    1,    2,    3,    2,    2,    1,    3,    2,
    2,    1,    3,    2,    1,    2,    1,    1,    3,    2,
    1,    0,    1,    1,
};
static const YYINT yydefred[] = {                         0,
    0,   23,   24,    0,    0,    3,    4,    8,   12,    0,
    0,    0,   15,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   17,   18,    0,    0,   13,    0,    1,   16,
    2,   21,    0,   20,
};
static const YYINT yydgoto[] = {                          4,
   14,   33,    5,   10,   11,   22,    6,   23,   24,
};
static const YYINT yysindex[] = {                      -254,
 -235,    0,    0,    0, -210,    0,    0,    0,    0, -239,
 -237, -231,    0, -212, -230, -231, -212, -234, -212, -201,
 -231, -209,    0,    0, -212, -207,    0, -206,    0,    0,
    0,    0, -208,    0,
};
static const YYINT yyrindex[] = {                         0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0, -251,    0, -225,    0, -222, -219,    0, -216, -245,
    0,    0,    0,    0, -213,    0,    0, -202,    0,    0,
    0,    0, -200,    0,
};
static const YYINT yygindex[] = {                         0,
  -11,    0,    0,   51,   54,   47,    0,  -20,    0,
};
#define YYTABLESIZE 65
static const YYINT yytable[] = {                         17,
   19,   30,    1,   17,   25,   30,    6,    6,    6,   28,
    2,    3,   14,   14,   14,   14,   14,   14,   12,    9,
    8,   16,   13,   12,   13,   21,    7,   13,   16,   21,
   13,   13,    7,    7,    7,   10,   10,   10,   11,   11,
   11,    5,    5,    5,    9,    9,    9,    8,    9,   20,
   21,   29,   21,   31,   34,   20,   32,   22,   22,   19,
   19,   18,   27,   15,   26,
};
static const YYINT yycheck[] = {                         11,
   12,   22,  257,   15,   16,   26,  258,  259,  260,   21,
  265,  266,  258,  259,  260,  261,  262,  263,  258,  259,
  258,  259,  262,  258,  262,  260,  262,  262,  259,  260,
  262,  262,  258,  259,  260,  258,  259,  260,  258,  259,
  260,  258,  259,  260,  258,  259,  260,  258,  259,  262,
  260,  261,  260,  261,  263,  262,  263,  260,  261,  260,
  261,   11,  264,   10,   18,
};
#define YYFINAL 4
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 266
#define YYUNDFTOKEN 278
#define YYTRANSLATE(a) ((a) > YYMAXTOKEN ? YYUNDFTOKEN : (a))
#if YYDEBUG
static const char *const yyname[] = {

"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"BLIF_MODEL","BLIF_INPUTS",
"BLIF_OUTPUTS","BLIF_NAMES","BLIF_END","NAME","GATE_COVER","LINE_CONT",
"BLIF_SEQ","BLIF_FSM",0,0,0,0,0,0,0,0,0,0,0,"illegal-symbol",
};
static const char *const yyrule[] = {
"$accept : circuit_module",
"circuit_module : decl_model decl_ip_list decl_op_list commandlist BLIF_END",
"circuit_module : decl_model decl_op_list decl_ip_list commandlist BLIF_END",
"circuit_module : invalid_ckt",
"decl_model : BLIF_MODEL NAME",
"decl_ip_list : decl_ip_list BLIF_INPUTS signal_list",
"decl_ip_list : decl_ip_list BLIF_INPUTS",
"decl_ip_list : decl_ip_list signal_list",
"decl_ip_list : BLIF_INPUTS",
"decl_op_list : decl_op_list BLIF_OUTPUTS signal_list",
"decl_op_list : decl_op_list BLIF_OUTPUTS",
"decl_op_list : decl_op_list signal_list",
"decl_op_list : BLIF_OUTPUTS",
"signal_list : signal_list NAME LINE_CONT",
"signal_list : signal_list NAME",
"signal_list : NAME",
"commandlist : commandlist command",
"commandlist : command",
"command : logic_gate",
"logic_gate : BLIF_NAMES signal_list cover_list",
"cover_list : cover_list GATE_COVER",
"cover_list : GATE_COVER",
"cover_list :",
"invalid_ckt : BLIF_SEQ",
"invalid_ckt : BLIF_FSM",

};
#endif

int      yydebug;
int      yynerrs;

int      yyerrflag;
int      yychar;
YYSTYPE  yyval;
YYSTYPE  yylval;

/* define the initial stack-sizes */
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH  YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 10000
#define YYMAXDEPTH  10000
#endif
#endif

#define YYINITSTACKSIZE 200

typedef struct {
    unsigned stacksize;
    YYINT    *s_base;
    YYINT    *s_mark;
    YYINT    *s_last;
    YYSTYPE  *l_base;
    YYSTYPE  *l_mark;
} YYSTACKDATA;
/* variables for the parser stack */
static YYSTACKDATA yystack;
#line 518 "blif.y"
extern FILE *yyin;

#ifdef STAND_ALONE
main(int argc, char **argv)
{
	FILE	*file;
#else
int ParseAndBuildCkt(FILE *file)
{
#endif
#ifdef NETLIST_DUMP
	char    output_file[] = "Output_Netlist";
#endif

#ifdef STAND_ALONE
	if (argc == 2){
		file = fopen(argv[1], "r");
		if (!file) {
			fprintf(stderr, "could not open %s\n", argv[1]);
			exit(1);
		}
	}
	else{
	    printf("Usage::  parse <filename>\n");
	    exit(0);
        }
#endif
		yyin = file;
#ifdef NETLIST_DUMP
		GateFile = fopen(output_file,"wt");
		if (!GateFile) {
			fprintf(stderr, "could not open %s\n", output_file);
			exit(1);
		}
#endif
	while (!feof(yyin)) {
		yyparse();
	}
  	Build_Ckt();
	printf("Circuit Parsed and Built successfully\n");
#ifdef STAND_ALONE
        fclose(file);
#endif
#ifdef NETLIST_DUMP
	fclose(GateFile);
#endif
        return 1;
}

yyerror(char *s)
{
	fprintf(stderr, "%s\n", s);
}

#ifdef DEBUG_MESSAGES 

void PrintGateInfo(void)
{
 int i;

 printf("Gate Type %s\n", gate_type[gateInfo.GateType].name); 
 for (i=0; i<gateInfo.NumOfNode-1; i++)
   printf("i/p name %s\n", gateInfo.NameOfNode[i]);
 printf("o/p name %s\n", gateInfo.NameOfNode[i]);
}

#endif

#ifdef NETLIST_DUMP
void DumpGateToFile(FILE *file, Gate_Info_t *gate)
{
  int i;

  fprintf(file, "%d\n", gate->GateType);
  /*  fprintf(file, "%s\n", gate_type[gate->GateType].name); */
  fprintf(file, "%d\n", gate->NumOfNode);
  for (i=0; i<gate->NumOfNode; i++)
    fprintf(file, "%s\n", gate->NameOfNode[i]);
  
}
#endif

void read_circuit(FILE *ckt_file)
{
  ParseAndBuildCkt(ckt_file);
} 

#line 434 "y.tab.c"

#if YYDEBUG
#include <stdio.h>		/* needed for printf */
#endif

#include <stdlib.h>	/* needed for malloc, etc */
#include <string.h>	/* needed for memset */

/* allocate initial stack or double stack size, up to YYMAXDEPTH */
static int yygrowstack(YYSTACKDATA *data)
{
    int i;
    unsigned newsize;
    YYINT *newss;
    YYSTYPE *newvs;

    if ((newsize = data->stacksize) == 0)
        newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
        return YYENOMEM;
    else if ((newsize *= 2) > YYMAXDEPTH)
        newsize = YYMAXDEPTH;

    i = (int) (data->s_mark - data->s_base);
    newss = (YYINT *)realloc(data->s_base, newsize * sizeof(*newss));
    if (newss == 0)
        return YYENOMEM;

    data->s_base = newss;
    data->s_mark = newss + i;

    newvs = (YYSTYPE *)realloc(data->l_base, newsize * sizeof(*newvs));
    if (newvs == 0)
        return YYENOMEM;

    data->l_base = newvs;
    data->l_mark = newvs + i;

    data->stacksize = newsize;
    data->s_last = data->s_base + newsize - 1;
    return 0;
}

#if YYPURE || defined(YY_NO_LEAKS)
static void yyfreestack(YYSTACKDATA *data)
{
    free(data->s_base);
    free(data->l_base);
    memset(data, 0, sizeof(*data));
}
#else
#define yyfreestack(data) /* nothing */
#endif

#define YYABORT  goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR  goto yyerrlab

int
YYPARSE_DECL()
{
    int yym, yyn, yystate;
#if YYDEBUG
    const char *yys;

    if ((yys = getenv("YYDEBUG")) != 0)
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = YYEMPTY;
    yystate = 0;

#if YYPURE
    memset(&yystack, 0, sizeof(yystack));
#endif

    if (yystack.s_base == NULL && yygrowstack(&yystack) == YYENOMEM) goto yyoverflow;
    yystack.s_mark = yystack.s_base;
    yystack.l_mark = yystack.l_base;
    yystate = 0;
    *yystack.s_mark = 0;

yyloop:
    if ((yyn = yydefred[yystate]) != 0) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = YYLEX) < 0) yychar = YYEOF;
#if YYDEBUG
        if (yydebug)
        {
            yys = yyname[YYTRANSLATE(yychar)];
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack) == YYENOMEM)
        {
            goto yyoverflow;
        }
        yystate = yytable[yyn];
        *++yystack.s_mark = yytable[yyn];
        *++yystack.l_mark = yylval;
        yychar = YYEMPTY;
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;

    YYERROR_CALL("syntax error");

    goto yyerrlab;

yyerrlab:
    ++yynerrs;

yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yystack.s_mark]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yystack.s_mark, yytable[yyn]);
#endif
                if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack) == YYENOMEM)
                {
                    goto yyoverflow;
                }
                yystate = yytable[yyn];
                *++yystack.s_mark = yytable[yyn];
                *++yystack.l_mark = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yystack.s_mark);
#endif
                if (yystack.s_mark <= yystack.s_base) goto yyabort;
                --yystack.s_mark;
                --yystack.l_mark;
            }
        }
    }
    else
    {
        if (yychar == YYEOF) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = yyname[YYTRANSLATE(yychar)];
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = YYEMPTY;
        goto yyloop;
    }

yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    if (yym)
        yyval = yystack.l_mark[1-yym];
    else
        memset(&yyval, 0, sizeof yyval);
    switch (yyn)
    {
case 1:
#line 158 "blif.y"
	{
#ifdef DEBUG_MESSAGES
		  printf("Circuit Model successfully parsed\n");
#endif 
		}
break;
case 2:
#line 164 "blif.y"
	{
#ifdef DEBUG_MESSAGES
		  printf("Circuit Model successfully parsed\n");
#endif 
		}
break;
case 3:
#line 170 "blif.y"
	{
#ifdef DEBUG_MESSAGES
		  printf("Not a Combinational Circuit\n");
#endif 
		}
break;
case 4:
#line 178 "blif.y"
	{ 
#ifdef DEBUG_MESSAGES
		  printf(".model %s parsed\n", yystack.l_mark[0].sval); 
#endif
		}
break;
case 5:
#line 186 "blif.y"
	{
#ifdef DEBUG_MESSAGES
                 printf(".input parsed - 4\n");
#endif
		 gate_type[PI].total++;
	        }
break;
case 6:
#line 193 "blif.y"
	{
#ifdef DEBUG_MESSAGES
                 printf(".input parsed - 3\n");
#endif 
		 gate_type[PI].total++;
		}
break;
case 7:
#line 200 "blif.y"
	{
#ifdef DEBUG_MESSAGES
		 printf(".input + signal_list parsed\n");
		 printf("Total PI's %d\n", gate_type[PI].total);
#endif 
		 signalNum =0;
		 gateInfo.GateType = nodeKind = UNKNOWN;		}
break;
case 8:
#line 208 "blif.y"
	{ 
#ifdef DEBUG_MESSAGES
		  printf(".input parsed\n"); 
#endif 
		 gateInfo.GateType = nodeKind = PI;
		 gateInfo.NumOfNode = 1;
		 gate_type[PI].total = 0;
		}
break;
case 9:
#line 220 "blif.y"
	{
#ifdef DEBUG_MESSAGES
                 printf(".output parsed - 1\n");
#endif 
		}
break;
case 10:
#line 226 "blif.y"
	{
#ifdef DEBUG_MESSAGES
                 printf(".output parsed - 2\n");
#endif 
		}
break;
case 11:
#line 232 "blif.y"
	{
#ifdef DEBUG_MESSAGES
		 printf(".output + signal_list parsed\n");
		 printf("Total PO's %d\n", gate_type[PO].total);
#endif 
		 signalNum =0;
		 gateInfo.GateType = nodeKind = UNKNOWN;		}
break;
case 12:
#line 240 "blif.y"
	{ 
#ifdef DEBUG_MESSAGES
		 printf(".output parsed\n"); 
#endif 
		 gateInfo.GateType = nodeKind = PO;
		 gateInfo.NumOfNode = 1;
		 gate_type[PO].total = 0;
		}
break;
case 13:
#line 252 "blif.y"
	{
#ifdef DEBUG_MESSAGES
		  printf("2 or more signals with line_cont parsed\n %dnd signal%s\n", signalNum + 1, yystack.l_mark[-1].sval);

#endif 
		  yyval.sval = yystack.l_mark[-1].sval;
		if((gateInfo.GateType == PI) || (gateInfo.GateType == PO)){ 
		  gateInfo.NameOfNode = (char **) malloc(
				    gateInfo.NumOfNode*sizeof(char *));
		  gateInfo.NameOfNode[0] = yystack.l_mark[-1].sval;
		  gate_type[gateInfo.GateType].total++;
#ifdef NETLIST_DUMP
		  DumpGateToFile(GateFile, &gateInfo);
#endif
                       Add_Gate(&gateInfo); 
		        free(gateInfo.NameOfNode);
		}
		 else {
		   if((signalArray = (char **) realloc(signalArray, 
					   (signalNum+1)*sizeof(char *)))
		                    == NULL)
		      perror("Realloc failed on line_cont signalArray\n");
		   signalArray[signalNum] = yystack.l_mark[-1].sval;
		   signalNum++;
		 }
		}
break;
case 14:
#line 279 "blif.y"
	{
#ifdef DEBUG_MESSAGES
		  printf("2 or more signal names parsed\n %dnd signal %s\n", 
			 signalNum+1, yystack.l_mark[0].sval);
#endif 
		  yyval.sval = yystack.l_mark[0].sval;
		if((gateInfo.GateType == PI) || (gateInfo.GateType == PO)){ 
		  gateInfo.NameOfNode = (char **) malloc(
			   gateInfo.NumOfNode*sizeof(char *));
		  gateInfo.NameOfNode[0] = yystack.l_mark[0].sval;
		  gate_type[gateInfo.GateType].total++;
#ifdef DEBUG_MESSAGES
		  PrintGateInfo();
#endif 
#ifdef NETLIST_DUMP
		  DumpGateToFile(GateFile, &gateInfo);		  
#endif
            Add_Gate(&gateInfo);
		   free(gateInfo.NameOfNode[0]);
		   free(gateInfo.NameOfNode);
		}
		 else {
		   int i;
#ifdef DEBUG_MESSAGES
		   printf("Before realloc\n");
		   for(i=0; i<signalNum; i++){
		     printf("%d signal: %s\n", i+1, 
			    signalArray[i]);
		   }
#endif 

		   if((signalArray = (char **) realloc(signalArray, 
					   (signalNum+1)*sizeof(char *))) 
		                                 == NULL)
		      perror("Realloc failed on signalArray\n");
		   signalArray[signalNum] = yystack.l_mark[0].sval;
#ifdef DEBUG_MESSAGES
		   printf("After realloc\n");
		   for(i=0; i<signalNum+1; i++){
		     printf("%d signal: %s\n", i+1, 
			    signalArray[i]);
		   }
#endif 
		   signalNum++;
		 }
		}
break;
case 15:
#line 326 "blif.y"
	{
#ifdef DEBUG_MESSAGES
		 printf("signal_name parsed %s\n", yystack.l_mark[0].sval);
#endif 
		 yyval.sval = yystack.l_mark[0].sval;
		 if((gateInfo.GateType == PI) || (gateInfo.GateType == PO)){
		   gateInfo.NameOfNode = (char **) malloc(
				gateInfo.NumOfNode*sizeof(char *));
		   gateInfo.NameOfNode[0] = yystack.l_mark[0].sval;
		   gate_type[gateInfo.GateType].total++;
#ifdef DEBUG_MESSAGES
		   PrintGateInfo();
#endif 
#ifdef NETLIST_DUMP
		   DumpGateToFile(GateFile, &gateInfo);		    
#endif
		     Add_Gate(&gateInfo); 
			 free(gateInfo.NameOfNode[0]);
		    free(gateInfo.NameOfNode);
		 }
		 else {  
#ifdef DEBUG_MESSAGES
		        printf("Gate Type %s\n", 
			       gate_type[gateInfo.GateType].name);
#endif 
		   if((signalArray = (char **) malloc((signalNum+1)*
				    sizeof(char *))) == NULL)
	                perror("malloc fail on signalArray\n");
		      else{
		 	signalArray[signalNum] = yystack.l_mark[0].sval;
#ifdef DEBUG_MESSAGES
			printf("Signal %s\n", signalArray[signalNum]);
#endif 
		      }

		 }
		 signalNum = 1;
                }
break;
case 19:
#line 375 "blif.y"
	{
                if(signalNum == 1){
                   if( yystack.l_mark[0].sval == (char *)0){
#ifdef DEBUG_MESSAGES
/*		       printf("CONSTANT 0 node %s\n", $2); */
		       printf("PO_GND node %s\n", yystack.l_mark[-1].sval);
#endif 
                		/* constant node 0 */
/*		         gateInfo.GateType = nodeKind = CONSTANT_0; */
		         gateInfo.GateType = nodeKind = PO_GND;
                   } 
                   else{
                    free(yystack.l_mark[0].sval);  /* freeing the cover storage for "1" */
#ifdef DEBUG_MESSAGES
/*			printf("CONSTANT 1 node %s\n", $2); */
			printf("PO_VCC node %s\n", yystack.l_mark[-1].sval);
#endif 
                		/* constant node 1 */
/*		 	gateInfo.GateType = nodeKind = CONSTANT_1; */
		 	gateInfo.GateType = nodeKind = PO_VCC;
                   }
		 	 gateInfo.NumOfNode = 1;		 
		         gateInfo.NameOfNode = signalArray;
		   	 gate_type[gateInfo.GateType].total++;
#ifdef DEBUG_MESSAGES
		         PrintGateInfo();
#endif 
#ifdef NETLIST_DUMP
			 DumpGateToFile(GateFile, &gateInfo);   
#endif
                         Add_Gate(&gateInfo); 
		   	  free(gateInfo.NameOfNode[0]);
		   	  free(gateInfo.NameOfNode);
                }
                else 
                if(signalNum > 1){
                int i;
				if(nodeKind != NAND && nodeKind != OR)
                    free(yystack.l_mark[0].sval);  /* freeing the cover storage */  
		 	gateInfo.NumOfNode = signalNum;		 
		   	gateInfo.NameOfNode = signalArray; 
		 	gateInfo.GateType = nodeKind;
			/* need to check for UNKNOWN gate and if it comes up
			 * may need to exit
			 */
#ifdef DEBUG_MESSAGES
			PrintGateInfo();
#endif 
#ifdef NETLIST_DUMP
			DumpGateToFile(GateFile, &gateInfo);
#endif
			if(signalNum > 3 || nodeKind == UNKNOWN){
				printf("Invalid Gate in Circuit\n");
				exit(-1);
			}
		    	  Add_Gate(&gateInfo); 

			  for(i=0; i<signalNum; i++){
		   		 free(gateInfo.NameOfNode[i]);
			  }
		   	 free(gateInfo.NameOfNode);
                }
#ifdef DEBUG_MESSAGES
		  printf("logic gate parsed\n");
#endif 
		  gateInfo.GateType = nodeKind = UNKNOWN;
		  signalNum = 0;
		  signalArray = NULL;

		}
break;
case 20:
#line 449 "blif.y"
	{
#ifdef DEBUG_MESSAGES
                  switch(nodeKind){
                    case OR:
		       printf("OR gate parsed\n%s\n%s\n", yystack.l_mark[-1].sval, yystack.l_mark[0].sval);
                    break;
                    case NAND:
		       printf("NAND gate parsed\n%s\n%s\n", yystack.l_mark[-1].sval, yystack.l_mark[0].sval);
                    break;
                  }
#endif 
                   free(yystack.l_mark[-1].sval);
                   free(yystack.l_mark[0].sval);

		}
break;
case 21:
#line 465 "blif.y"
	{
		  int i;

		  yyval.sval = yystack.l_mark[0].sval;

		  /***************************************************\ 
		   * check till only gates that actually have a cover
		   * such as 
		   *			.names [1]  or  .names [2] [4] [5]
 		   *			1 				11 1
		   * These gates are gates 0 (AND) to 5 (BUF) and
		   * node PO_VCC
		   * PO_GND appears as follows 
		   *			.names [1]  
		   *
		   * This is because we decide the logic gate by actually
		   * doing a table lookup (compare the cover with the
		   * one in the table to decide the nodeKind)
		   \***************************************************/
/*		  for(i=0; i<MAX_GATE_TYPES-4; i++){ */
		  for(i=0; i<MAX_GATE_TYPES-5; i++){
		  	if(!strcmp(funcCover, gate_type[i].cover)){
#ifdef DEBUG_MESSAGES
		   	    printf("%s gate %s\n", gate_type[i].name, yystack.l_mark[0].sval);
#endif 
		   	    nodeKind = i;
		   	    break;
		  	}
		  }
		  if(i == MAX_GATE_TYPES - 5){ /* ie loop exits wo breaking */
		  	if(!strcmp(funcCover, gate_type[PO_VCC].cover)){
#ifdef DEBUG_MESSAGES
		   	    printf("%s gate %s\n", gate_type[i].name, yystack.l_mark[0].sval);
#endif 
				nodeKind = PO_VCC;
			}
		  }
		}
break;
case 22:
#line 504 "blif.y"
	{ yyval.sval = (char *) 0; }
break;
case 23:
#line 508 "blif.y"
	{
					printf("Sequential ckt\n");
				}
break;
case 24:
#line 512 "blif.y"
	{
					printf("FSM ckt\n");
				}
break;
#line 1011 "y.tab.c"
    }
    yystack.s_mark -= yym;
    yystate = *yystack.s_mark;
    yystack.l_mark -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yystack.s_mark = YYFINAL;
        *++yystack.l_mark = yyval;
        if (yychar < 0)
        {
            if ((yychar = YYLEX) < 0) yychar = YYEOF;
#if YYDEBUG
            if (yydebug)
            {
                yys = yyname[YYTRANSLATE(yychar)];
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == YYEOF) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yystack.s_mark, yystate);
#endif
    if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack) == YYENOMEM)
    {
        goto yyoverflow;
    }
    *++yystack.s_mark = (YYINT) yystate;
    *++yystack.l_mark = yyval;
    goto yyloop;

yyoverflow:
    YYERROR_CALL("yacc stack overflow");

yyabort:
    yyfreestack(&yystack);
    return (1);

yyaccept:
    yyfreestack(&yystack);
    return (0);
}
