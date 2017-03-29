#include "project.h"
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>


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

#define compute_NOR(result,val1,val2) \
  { \
    switch (val1) { \
    case LOGIC_0: \
      if ( val2 == LOGIC_1 ) result = LOGIC_0; \
      else { \
        if ( val2 == LOGIC_0 ) result = LOGIC_1; \
        else result = LOGIC_X; } \
      break; \
    case LOGIC_1: \
      result = LOGIC_0; \
      break; \
    case LOGIC_X: \
      if ( val2 == LOGIC_1 ) result = LOGIC_0; \
      else result = LOGIC_X; \
      break; \
    default: \
      assert(0); \
    } \
  } \

#define compute_NAND(result,val1,val2) \
    switch (val1) { \
    case LOGIC_0: \
      result = LOGIC_1; \
      break; \
    case LOGIC_1: \
      if ( val2 == LOGIC_1 ) result = LOGIC_0; \
      else { \
        if ( val2 == LOGIC_0 ) result = LOGIC_1; \
        else result = LOGIC_X; } \
      break; \
    case LOGIC_X: \
      if ( val2 == LOGIC_0 ) result = LOGIC_1; \
      else result = LOGIC_X; \
      break; \
    default: \
      assert(0); \
    } \

#define evaluate(gate) \
  { \
    switch( gate.type ) { \
    case NAND: \
      compute_NAND(gate.out_val,gate.in_val[0],gate.in_val[1]); \
      break; \
    case NOR: \
      compute_NOR(gate.out_val,gate.in_val[0],gate.in_val[1]); \
      break; \
    case AND: \
      compute_AND(gate.out_val,gate.in_val[0],gate.in_val[1]); \
      break; \
    case OR: \
      compute_OR(gate.out_val,gate.in_val[0],gate.in_val[1]); \
      break; \
    case INV: \
      compute_INV(gate.out_val,gate.in_val[0]); \
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
    case PI: \
      break; \
    default: \
      assert(0); \
    } \
  }

/*************************************************************************

Function:  three_val_fault_simulate

Purpose:  This function performs fault simulation on 3-valued input patterns.

pat.out[][] is filled with the fault-free output patterns corresponding to
the input patterns in pat.in[][].

Return:  List of faults that remain undetected.

*************************************************************************/
//
// The overall strategy for this function is to execute as few
// instructions as possible. This is accomplished in three ways:
// 1) Utilize a differential algorithm to minimize the number
//    of times the circuit must be put into a fault-free state.
// 2) Only re-evaluate the portions of a circuit affected by the
//    injection of a fault.
// 3) Determine as quickly as possible when a fault will NOT be detected,
//    and immediately exit and load the next fault. This strategy turned
//    out to yield the biggest performance gain. The idea is simple - if
//    the output of a faulty gate is the same as the output of the same
//    gate in a non-faulty state, then the fault will not be detected.
//
// The data structures used are the same basic data structures as in the
// original sequential implementation, with the addition of two fields to
// the gate-t structure. One of these fields, out_saved, is used to save
// the fault-free output value of a gate, and enables restoring the state
// in the differential algorithm, as well as optimization (3) above. The
// other field, gclass, saves the class of a gate. A gate class is defined
// as an enum type having the value INPUT_2, INPUT_1, or INPUT_0, and this
// streamlines the loading of gate input values based on number of inputs
// of the gate.

fault_list_t *
three_val_fault_simulate(
     circuit_t    *ckt,
     pattern_t    *pat,
     fault_list_t *undetected_flist
     )
{
    int p;  // looping variable for pattern number
    int i;
    int j;  // current fault index
    int k;  // previous fault index
    fault_list_t *fptr, *prev_fptr, *collapsing_fptr;
    int detected_flag = FALSE;
    int noFault       = FALSE;


    for (p = 0; (p < pat->len); p++) {

         // assign primary input values for pattern
        for (i = 0; i < ckt->npi; i++) {
            ckt->gate[ckt->pi[i]].out_val = pat->in[p][i];
        }

         // evaluate all gates
        for (i = 0; i < ckt->ngates; i++) {

            switch (ckt->gate[i].type) {
                case NAND :
                case NOR  :
                case AND  :
                case OR   :
                     // gates with two input terminals
                    ckt->gate[i].gclass    = INPUT_2;
                    ckt->gate[i].in_val[0] = ckt->gate[ckt->gate[i].fanin[0]].out_val;
                    ckt->gate[i].in_val[1] = ckt->gate[ckt->gate[i].fanin[1]].out_val;
                    break;
                case INV :
                case BUF :
                case PO  :
                     // gates with one input terminal
                    ckt->gate[i].gclass    = INPUT_1;
                    ckt->gate[i].in_val[0] = ckt->gate[ckt->gate[i].fanin[0]].out_val;
                    break;
                default:
                    ckt->gate[i].gclass = INPUT_0;
            }

             // compute gate output value
            evaluate(ckt->gate[i]);

             // save state of gate
            ckt->gate[i].out_saved = ckt->gate[i].out_val;
        }  // end-for

         // put fault-free primary output values into pat data structure
        for (i = 0; i < ckt->npo; i++) {
            pat->out[p][i] = ckt->gate[ckt->po[i]].out_val;
        }

         //--------------------------------------

         // now that the pattern is setup in a fault-free state, loop
         // thru each fault and see if the pattern can detect it - this is
         // a basic component of the differential algorithm
        j = -1;
        prev_fptr = (fault_list_t *)NULL;
        for (fptr=undetected_flist; fptr != (fault_list_t *)NULL; fptr=fptr->next) {

            k = j;
            j = fptr->gate_index;

             // get lower limit on gate eval
            if ((j < k) || (k < 0)) {
                k = j;
            }
          
             // evaluate all effected gates - the gates prior to this are already
             // in a fault-free state, and do not need re-evaluating - this is 
             // optimization (2)
            for (i = k; i < ckt->ngates; i++) {

                 // restore state of gate - differential algorithm
                ckt->gate[i].out_val   = ckt->gate[i].out_saved;

                 // get gate input values
                switch ( ckt->gate[i].gclass ) {
                    case INPUT_2 :
                         // gates with two input terminals
                        ckt->gate[i].in_val[0] = ckt->gate[ckt->gate[i].fanin[0]].out_val;
                        ckt->gate[i].in_val[1] = ckt->gate[ckt->gate[i].fanin[1]].out_val;
                        break;
                    case INPUT_1 :
                         // gates with one input terminal
                        ckt->gate[i].in_val[0] = ckt->gate[ckt->gate[i].fanin[0]].out_val;
                        break;
                }

                 // check if faulty gate
                if (i == j) {
                    noFault = FALSE;

                     // check if fault at input
                    if (fptr->input_index >= 0) {
                         // inject fault
                        if (fptr->type == S_A_0) {
                            ckt->gate[i].in_val[fptr->input_index] = LOGIC_0;
                        }
                        else {
                             // S_A_1
                            ckt->gate[i].in_val[fptr->input_index] = LOGIC_1;
                        }
                         // compute gate output value
                        evaluate(ckt->gate[i]);
                    }  // end-if
                    else {
                         // fault at output
                        evaluate(ckt->gate[i]);
                         // inject fault
                        if (fptr->type == S_A_0) {
                            ckt->gate[i].out_val = LOGIC_0;
                        }
                        else {
                             // S_A_1
                            ckt->gate[i].out_val = LOGIC_1;
                        }

                    }
                     // see if the output of this faulty gate changed from its
                     // fault-free state - if not then this fault cannot be
                     // detected by this pattern - this is optimization (3)
                    if (ckt->gate[i].out_val == ckt->gate[i].out_saved) {
                       // output did not change - this fault has no effect
                      noFault = TRUE;
                       // exit the for-loop
                      break;
                    }
                }  // end-if
                else {
                     // not faulty gate - compute gate output value
                    evaluate(ckt->gate[i]);
                }
            }  // end-for

             // check if fault detected
            if (FALSE == noFault) {
                for (i = 0; i < ckt->npo; i++) {
                    if ((ckt->gate[ckt->po[i]].out_val == LOGIC_0) && (pat->out[p][i] == LOGIC_1)) {
                        detected_flag = TRUE;
                        break;
                    }
                    else {
                        if ((ckt->gate[ckt->po[i]].out_val == LOGIC_1) && (pat->out[p][i] == LOGIC_0)) {
                            detected_flag = TRUE;
                            break;
                        }
                    }
                }  // end-for
            }  // end-if

            if (detected_flag) {
                  // remove fault from undetected fault list
                if (fptr == undetected_flist) {
                     // if first fault in fault list, advance head of list pointer
                    undetected_flist = fptr->next;
                    prev_fptr        = fptr->next;
                }
                else {
                     // if not first fault in fault list, then remove link
                    prev_fptr->next = fptr->next;
                }

// ALVIN'S CODE FAULT COLLAPSING              
                if(fptr->input_index == -1)
                {
                    collapsing_fptr = fptr->next;
                    switch(ckt->gate[fptr->gate_index].type)
                    {
                        case AND:
                           if(fptr->type == S_A_0)
                           { 
                               for(i = 0; i < 5 && collapsing_fptr != NULL; i++)
                               {
                                    if(collapsing_fptr->gate_index != fptr->gate_index){
                                        break;
                                    }
                                    else if(collapsing_fptr->input_index >= 0 && collapsing_fptr->type == S_A_0)
                                    {
                                        if (collapsing_fptr == undetected_flist) 
                                        {
                                            // if first fault in fault list, advance head of list pointer
                                            undetected_flist = collapsing_fptr->next;
                                            prev_fptr        = collapsing_fptr->next;
                                        }                                   
                                        else 
                                        {
                                            // if not first fault in fault list, then remove link
                                            prev_fptr->next = collapsing_fptr->next;
                                        }                                                         
                                    }
                                    collapsing_fptr = collapsing_fptr->next;
                               }
                           } 
                        break;
                        case NAND:                          
                           if(fptr->type == S_A_1)
                           { 
                               for(i = 0; i < 5 && collapsing_fptr != NULL; i++)
                               {
                                    if(collapsing_fptr->gate_index != fptr->gate_index){
                                        break;
                                    }
                                    else if(collapsing_fptr->input_index >= 0 && collapsing_fptr->type == S_A_0)
                                    {
                                        if (collapsing_fptr == undetected_flist) 
                                        {
                                            // if first fault in fault list, advance head of list pointer
                                            undetected_flist = collapsing_fptr->next;
                                            prev_fptr        = collapsing_fptr->next;
                                        }                                   
                                        else 
                                        {
                                            // if not first fault in fault list, then remove link
                                            prev_fptr->next = collapsing_fptr->next;
                                        }                                                         
                                    }
                                    collapsing_fptr = collapsing_fptr->next;
                               }
                           }                             
                        break;
                        case OR:
                           if(fptr->type == S_A_1)
                           { 
                               for(i = 0; i < 5 && collapsing_fptr != NULL; i++)
                               {
                                    if(collapsing_fptr->gate_index != fptr->gate_index){
                                        break;
                                    }
                                    else if(collapsing_fptr->input_index >= 0 && collapsing_fptr->type == S_A_1)
                                    {
                                        if (collapsing_fptr == undetected_flist) 
                                        {
                                            // if first fault in fault list, advance head of list pointer
                                            undetected_flist = collapsing_fptr->next;
                                            prev_fptr        = collapsing_fptr->next;
                                        }                                   
                                        else 
                                        {
                                            // if not first fault in fault list, then remove link
                                            prev_fptr->next = collapsing_fptr->next;
                                        }                                                         
                                    }
                                    collapsing_fptr = collapsing_fptr->next;
                               }
                           }                            
                        break;
                        case NOR:
                           if(fptr->type == S_A_0)
                           { 
                               for(i = 0; i < 5 && collapsing_fptr != NULL; i++)
                               {
                                    if(collapsing_fptr->gate_index != fptr->gate_index){
                                        break;
                                    }
                                    else if(collapsing_fptr->input_index >= 0 && collapsing_fptr->type == S_A_1)
                                    {
                                        if (collapsing_fptr == undetected_flist) 
                                        {
                                            // if first fault in fault list, advance head of list pointer
                                            undetected_flist = collapsing_fptr->next;
                                            prev_fptr        = collapsing_fptr->next;
                                        }                                   
                                        else 
                                        {
                                            // if not first fault in fault list, then remove link
                                            prev_fptr->next = collapsing_fptr->next;
                                        }                                                         
                                    }
                                    collapsing_fptr = collapsing_fptr->next;
                               }
                           }                            
                        break;
                        default:
                        break;
                    }
                }

                detected_flag = FALSE;
            }
            else {
                 // fault remains undetected, keep on list
                prev_fptr = fptr;
            }
        }  // end-for
    }  // end-for

    return(undetected_flist);

}
