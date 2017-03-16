  
%{
/******************************************************************\ 
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
 * The project is Path Delay Fault simulation. This file is a "lex" 
 * file that creates the "lex.yy.c" file which contains the 
 * token scanner used to read the  BLIF file that has 
 * been mapped to a technology library containing only 2-input or less
 * combinational gates. The scanner communicates with the "yacc"
 * file to parse  the BLIF file 
\******************************************************************/


/*
 * Definition Section
 */

#include "y.tab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/******************************************************************\
 * nameBuf: passes the pointer to the signal name to yacc
 * funcCover: passes the pointer to the function cover to yacc
\******************************************************************/
char *nameBuf;
char *funcCover;

%}

COMMENT                     ^#[^\n]*\n
WHITESPACE		    [ \n\t]
ANYNAME			    [^ \t\n\\]+
COVER			    ^[01-]*[ \t\n]+[01-]*
DECIMALNO			(([0-9]*\.[0-9]+)([ \t]))+

%%

{WHITESPACE}		;
{COMMENT}               ;

\.model			{ return (BLIF_MODEL); }	
\.inputs                { return (BLIF_INPUTS); }
\.outputs               { return (BLIF_OUTPUTS); }
\.names 		{ return (BLIF_NAMES); }
\.end			{ return (BLIF_END); }
\\ 			{ return (LINE_CONT); }
\.clock                 { return (BLIF_SEQ); }
\.exdc                  ;
\.subckt                ;
\.latch                 { return (BLIF_SEQ); }
\.gate                  ;
\.mlatch                ;
\.search                ;
\.start_kiss            { return (BLIF_FSM); }
\.latch_order           { return (BLIF_SEQ); }
\.cycle                 { return (BLIF_SEQ); }
\.clock_event           { return (BLIF_SEQ); }
\.code                  { return (BLIF_FSM); }
\.area                  							;
\.delay                 							;
\.wire_load_slope                  					;
\.wire                  							;
\.input_arrival         							;
\.default_input_arrival[ \t]+{DECIMALNO}			;
\.output_required[ \t]+{DECIMALNO}     				;
\.default_output_required[ \t]+{DECIMALNO}			;
\.input_drive[ \t]+{DECIMALNO}           			;
\.default_input_drive[ \t]+{DECIMALNO}   			;
\.output_load[ \t]+{DECIMALNO}           			;
\.default_output_load[ \t]+{DECIMALNO}   			;
\.default_max_input_load[ \t]+{DECIMALNO}    		;


{ANYNAME}		{
                          nameBuf = (char *)malloc((yyleng+1)*sizeof(char));
                          strcpy(nameBuf, yytext);
                          yylval.sval = nameBuf;
                          return (NAME); 
                        }
{COVER}			{ 
                          funcCover = (char *)malloc((yyleng+1)*sizeof(char));
                          strcpy(funcCover, yytext);
                          yylval.sval = funcCover;
                          return (GATE_COVER); 
                        }

.			;

%%
