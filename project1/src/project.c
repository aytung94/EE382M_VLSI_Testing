#include "project.h"
#include <stdio.h>
#include <assert.h>

#define PRINT 1

/* Macro Definitions */

#define compute_INV(result,val) \
  { \
    switch (val) { \
    case LOGIC_0: \
      result = LOGIC_1; \
      break; \
    case LOGIC_1: \
      result = LOGIC_0; \
      break; \
    case LOGIC_X: \
      result = LOGIC_X; \
      break; \
    default: \
      assert(0); \
    } \
  }

#define compute_AND(result,val1,val2) \
    switch (val1) { \
    case LOGIC_0: \
      result = LOGIC_0; \
      break; \
    case LOGIC_1: \
      result = val2; \
      break; \
    case LOGIC_X: \
      if ( val2 == LOGIC_0 ) result = LOGIC_0; \
      else result = LOGIC_X; \
      break; \
    default: \
      assert(0); \
    } 

#define compute_OR(result,val1,val2) \
  { \
    switch (val1) { \
    case LOGIC_1: \
      result = LOGIC_1; \
      break; \
    case LOGIC_0: \
      result = val2; \
      break; \
    case LOGIC_X: \
      if ( val2 == LOGIC_1 ) result = LOGIC_1; \
      else result = LOGIC_X; \
      break; \
    default: \
      assert(0); \
    } \
  }

#define evaluate(gate) \
  { \
    switch( gate.type ) { \
    case PI: \
      break; \
    case PO: \
    case BUF: \
      gate.out_val = gate.in_val[0]; \
      break; \
    case PO_GND: \
      gate.out_val = LOGIC_0; \
      break; \
    case PO_VCC: \
      gate.out_val = LOGIC_1; \
      break; \
    case INV: \
      compute_INV(gate.out_val,gate.in_val[0]); \
      break; \
    case AND: \
      compute_AND(gate.out_val,gate.in_val[0],gate.in_val[1]); \
      break; \
    case NAND: \
      compute_AND(gate.out_val,gate.in_val[0],gate.in_val[1]); \
      compute_INV(gate.out_val,gate.out_val); \
      break; \
    case OR: \
      compute_OR(gate.out_val,gate.in_val[0],gate.in_val[1]); \
      break; \
    case NOR: \
      compute_OR(gate.out_val,gate.in_val[0],gate.in_val[1]); \
      compute_INV(gate.out_val,gate.out_val); \
      break; \
    default: \
      assert(0); \
    } \
  }

#define print_fault_list_t(flist) \
  { \
    fault_list_t *current = undetected_flist; \
    while(current != NULL) \
    { \
      printf("%d.%d/%d\n", current->gate_index, current->input_index, (int)current->type); \
      current = current->next; \
    } \
  } 
  
/*************************************************************************

Function:  three_val_transition_fault_simulate

Purpose:  This function performs transition fault simulation on 3-valued
          input patterns.

pat.out[][] is filled with the fault-free output patterns corresponding to
the input patterns in pat.in[][].

Return:  List of faults that remain undetected.

*************************************************************************/

fault_list_t *three_val_fault_simulate(ckt,pat,undetected_flist)
     circuit_t *ckt;
     pattern_t *pat;
     fault_list_t *undetected_flist;
{

  /**************************/
  /* print gate information */
  /**************************/

#if (PRINT == 1)
  printf("Gate Information\n");
  for(int i = 0; i < ckt->ngates; i++)
  {
    gate_t cur = ckt->gate[i];
    if((int)cur.type >= AND && (int)cur.type <= NOR)
    {  
      printf("Name: %s\n-Index: %d\n-Gate: %d\n-Inputs: %d,%d\n\n", cur.name, cur.index, (int)cur.type, cur.fanin[0], cur.fanin[1]);
    }
    else if((int)cur.type == INV || (int)cur.type == BUF)
    {
      printf("Name: %s\n-Index: %d\n-Gate: %d\n-Inputs: %d\n\n", cur.name, cur.index, (int)cur.type, cur.fanin[0]);
    }   
    else if((int)cur.type == PI)
    {
       printf("Name: %s\n-Index: %d\n-Gate: %d\n-Inputs: %d\n\n", cur.name, cur.index, (int)cur.type, cur.fanin[0]);
    }
  }
  
  printf("Fault List Information\n");
  print_fault_list_t(undetected_flist);
  
#endif

  /* put your code here */
  
  int p;  /* looping variable for pattern number */
  int i;
  
  /*************************/
  /* fault-free simulation */
  /*************************/   

// TODO: remove all stuck at faults if out is the same, only evaluate if value changes per gate
  
  /* loop through all patterns */
  for (p = 0; p < pat->len; p++) {
    /* initialize all gate values to UNDEFINED */
    for (i = 0; i < ckt->ngates; i++) {
      ckt->gate[i].in_val[0] = UNDEFINED;
      ckt->gate[i].in_val[1] = UNDEFINED;
      ckt->gate[i].out_val = UNDEFINED;
    }
    /* assign primary input values for pattern */
    for (i = 0; i < ckt->npi; i++) {
      ckt->gate[ckt->pi[i]].out_val = pat->in[p][i];
    }
    /* evaluate all gates */
    for (i = 0; i < ckt->ngates; i++) {
      /* get gate input values */
      switch ( ckt->gate[i].type ) {
      /* gates with no input terminal */
      case PI:
      case PO_GND:
      case PO_VCC:
	break;
      /* gates with one input terminal */
      case INV:
      case BUF:
      case PO:
	ckt->gate[i].in_val[0] = ckt->gate[ckt->gate[i].fanin[0]].out_val;
	break;
      /* gates with two input terminals */
      case AND:
      case NAND:
      case OR:
      case NOR:
	ckt->gate[i].in_val[0] = ckt->gate[ckt->gate[i].fanin[0]].out_val;
	ckt->gate[i].in_val[1] = ckt->gate[ckt->gate[i].fanin[1]].out_val;
	break;
      default:
	assert(0);
      }
      /* compute gate output value */
      evaluate(ckt->gate[i]);
    }
    /* put fault-free primary output values into pat data structure */
    for (i = 0; i < ckt->npo; i++) {
      pat->out[p][i] = ckt->gate[ckt->po[i]].out_val;
    }
  }
    
  /********************/
  /* fault simulation */
  /********************/
    
  return(undetected_flist);
}
