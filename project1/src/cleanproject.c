#include "project.h"
#include <stdio.h>
#include <assert.h>

#define PRINT 0

/* Macro Definitions */

#define compute_INV(result,val) \
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
  
#define evaluate_diff(gate, val0, val1, temp, diff) \
  { \
    switch( gate->type ) { \
    case PI: \
      diff = 0; \
      break; \
    case PO: \
    case BUF: \
      temp = val0; \
      diff = (gate->out_val != val0); \
      break; \
    case PO_GND: \
      diff = 0; \
      break; \
    case PO_VCC: \
      diff = 0; \
      break; \
    case INV: \
      compute_INV(temp,val0); \
      diff = (gate->out_val != temp); \
      break; \
    case AND: \
      compute_AND(temp,val0,val1); \
      diff = (gate->out_val != temp); \
      break; \
    case NAND: \
      compute_AND(temp,val0,val1); \
      compute_INV(temp,temp); \
      diff = (gate->out_val != temp); \
      break; \
    case OR: \
      compute_OR(temp,val0,val1); \
      diff = (gate->out_val != temp); \
      break; \
    case NOR: \
      compute_OR(temp,val0,val1); \
      compute_INV(temp,temp); \
      diff = (gate->out_val != temp); \
      break; \
    default: \
      assert(0); \
      break; \
    } \
  }  
  
#if (PRINT == 1)  
#define print_assign_fault(saf) \
  {\
    printf("+"); \
    if(saf == LOGIC_0) \
      printf("--S@0 fault\n"); \
    else \
      printf("--S@1 fault\n"); \
  }
#else
#define print_assign_fault(saf)
#endif
  
#define assign_fault(cur, io, saf, co, m) \
  { \
    cur->faulty_gates[cur->num_faulty_gates].gate_index = cur->index; \
    cur->faulty_gates[cur->num_faulty_gates].input_index = io; \
    cur->faulty_gates[cur->num_faulty_gates].type = saf; \
    cur->faulty_gates[cur->num_faulty_gates].next = NULL; \
    cur->faulty_gates[cur->num_faulty_gates].concur_out = co; \
    cur->faulty_gates[cur->num_faulty_gates].mark = m; \
    cur->num_faulty_gates++; \
    print_assign_fault(saf); \
  }            

#define assign_fault_to(cur, io, saf, co, m, append_to_me_ind) \
   { \
    cur->faulty_gates[cur->num_faulty_gates].gate_index = cur->index; \
    cur->faulty_gates[cur->num_faulty_gates].input_index = io; \
    cur->faulty_gates[cur->num_faulty_gates].type = saf; \
    cur->faulty_gates[cur->num_faulty_gates].next = NULL; \
    cur->faulty_gates[cur->num_faulty_gates].concur_out = co; \
    cur->faulty_gates[cur->num_faulty_gates].mark = m; \
    cur->num_faulty_gates++; \
    print_assign_fault(saf); \
  } 
  
#define print_fault_list_t(flist) \
  { \
    fault_list_t *current = undetected_flist; \
    while(current != NULL) \
    { \
      printf("%d.%d/%d\n", current->gate_index, current->input_index, current->type); \
      current = current->next; \
    } \
  } 

#define search(find_me, in_this, found, count) \
  { \
    found = 0; \
    for(count = 0; count < in_this->num_faulty_gates; count++) \
    { \
      if(find_me.gate_index == in_this->faulty_gates[count].gate_index && find_me.input_index == in_this->faulty_gates[count].input_index && find_me.type == in_this->faulty_gates[count].type) \
      { \
         found = 1; \
         break; \
      } \
    } \
  }  
  
#define ID1(cur) (cur.gate_index*6 + cur.type*3 + (cur.input_index + 1))
#define ID2(gi, ct, ii) (gi*6 + ct*3 + (ii + 1))

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
  int i;

#if (PRINT == 1)
  printf("Gate Information\n");
  for(i = 0; i < ckt->ngates; i++)
  {
    gate_t* cur = &ckt->gate[i];
    switch(cur->type)
    {
      case AND:
      case OR:
      case NAND:
      case NOR:
        printf("Name: %s\n-Index: %d\n-Gate: %d\n-Inputs: %d,%d\n-InpVal: %d,%d\n-OutVal: %d\n\n", cur->name, cur->index, cur->type, cur->fanin[0], cur->fanin[1], cur->in_val[0], cur->in_val[1], cur->out_val);
        break;
      case INV:
      case BUF:
        printf("Name: %s\n-Index: %d\n-Gate: %d\n-Inputs: %d\n-InpVal: %d\n-OutVal: %d\n\n", cur->name, cur->index, cur->type, cur->fanin[0], cur->in_val[0], cur->out_val);
        break;
      case PI:
        printf("Name: %s\n-Index: %d\n-Gate: %d\n-OutVal: %d\n\n", cur->name, cur->index, cur->type, cur->out_val);     
        break;
      case PO:
        printf("Name: %s\n-Index: %d\n-Gate: %d\n-Inputs: %d\n-InpVal: %d\n\n", cur->name, cur->index, cur->type, cur->fanin[0], cur->in_val[0]);
        break;
      case PO_GND:
      case PO_VCC:
        printf("Name: %s\n-Index: %d\n-Gate: %d\n-Inputs: %d\n\n", cur->name, cur->index, cur->type, cur->fanin[0]);      
        break;
      default:
        assert(0);
    }   
  }
  
  printf("Fault List Information\n");
  print_fault_list_t(undetected_flist);  
#endif

  /* put your code here */
  
  int p;  /* looping variable for pattern number */
  int done = 0;
  
  /*************************/
  /* fault-free simulation */
  /*************************/   

// TODO: remove all stuck at faults if out is the same, only evaluate if value changes per gate
  
  int faults_detected[MAX_FAULTS] = {0};
  /* loop through all patterns */
  for (p = 0; p < pat->len; p++) {
            
    /* initialize all gate values to UNDEFINED TODO UNNECESSARY
    for (i = 0; i < ckt->ngates; i++) {
      ckt->gate[i].in_val[0] = UNDEFINED;
      ckt->gate[i].in_val[1] = UNDEFINED;
      ckt->gate[i].out_val = UNDEFINED;
    }*/
    /* assign primary input values for pattern 
    for (i = 0; i < ckt->npi; i++) {
      ckt->gate[ckt->pi[i]].out_val = pat->in[p][i];
    }*/
    /* evaluate all gates */
    for (i = 0; i < ckt->ngates; i++) {
      ckt->gate[i].num_faulty_gates = 0;     
      /* get gate input values */

      switch ( ckt->gate[i].type ) {
      /* gates with no input terminal */
      case PI:
        ckt->gate[ckt->pi[i]].out_val = pat->in[p][i];      
      //  ckt->gate[i].num_faulty_gates = 0;
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
//  }
    
  /********************/
  /* fault simulation */
  /********************/
  if(!done)  
  {
  //  int pat_len = pat->len;
    int ckt_ngates = ckt->ngates;
    int ckt_npo = ckt->npo;
    gate_t *gate = ckt->gate;   
 
#if (PRINT == 1)
  printf("\nPATTERN: %d\n", p);      
#endif  

    int g;
    int fcount;
    gate_t* cur;
      gate_t* inp0;
      gate_t* inp1;        
      //cur->num_faulty_gates = 0;
      int val0; 
      int val1;
      int temp;
      int diff;  
      int found;
      int inp0_fcount;
      int inp1_fcount;   
    int o;
    int f;
    
    for(g = 0; g < ckt_ngates; g++)
    {           
      // Check input gates for bad gate lists
      cur = &ckt->gate[g];
      
#if (PRINT == 1)       
      printf("Gate: %d\n", cur->index);
#endif      
      switch(cur->type)
      {
        // one input gate
        case INV:
        case BUF:
          inp0 = &ckt->gate[cur->fanin[0]];
          for(fcount = 0; fcount < inp0->num_faulty_gates; fcount++)
          {           
            evaluate_diff(cur, inp0->faulty_gates[fcount].concur_out, cur->in_val[1], temp, diff);                          
            if(diff)
            {
              cur->faulty_gates[cur->num_faulty_gates] = inp0->faulty_gates[fcount];
              cur->faulty_gates[cur->num_faulty_gates].concur_out = temp; 
              cur->num_faulty_gates++;    
#if (PRINT == 1)                     
              printf("+");
#endif         
            }                    
          }  
#if (PRINT == 1)                 
          printf("-First Input Gate Check\n");          
#endif                        

          break;         
        case PO:
          inp0 = &ckt->gate[cur->fanin[0]];
          for(fcount = 0; fcount < inp0->num_faulty_gates; fcount++)
          {           
            evaluate_diff(cur, inp0->faulty_gates[fcount].concur_out, cur->in_val[1], temp, diff);                                        
            if(diff && temp != LOGIC_X && cur->out_val != LOGIC_X)
            {
              cur->faulty_gates[cur->num_faulty_gates] = inp0->faulty_gates[fcount];
              cur->faulty_gates[cur->num_faulty_gates].concur_out = temp; 
              cur->num_faulty_gates++;    
#if (PRINT == 1)                     
              printf("+");
#endif                             
            }                       
          }  
#if (PRINT == 1)                 
          printf("-First Input Gate Check\n");          
#endif                        
          break;          
        // two input gate
        case AND:
        case OR:
        case NAND:
        case NOR: 
          inp0 = &ckt->gate[cur->fanin[0]];
          inp1 = &ckt->gate[cur->fanin[1]];
          for(fcount = 0; fcount < inp0->num_faulty_gates; fcount++)
          {
            // TODO SEARCH FOR SAME FAULT to combine
            search(inp0->faulty_gates[fcount], inp1, found, inp1_fcount);
            if(found){                                
              evaluate_diff(cur, inp0->faulty_gates[fcount].concur_out, inp1->faulty_gates[inp1_fcount].concur_out, temp, diff);
#if (PRINT == 1) 
                printf("f");
#endif                
                if(diff)
                {
#if (PRINT == 1)
                  printf("+");
#endif                  
                  cur->faulty_gates[cur->num_faulty_gates] = inp0->faulty_gates[fcount];
                  cur->faulty_gates[cur->num_faulty_gates].concur_out = temp;
                  cur->num_faulty_gates++;
                }                       
                // if found, mark seen
              inp1->faulty_gates[inp1_fcount].mark = 1;
            }
            else{             
              evaluate_diff(cur, inp0->faulty_gates[fcount].concur_out, cur->in_val[1], temp, diff);    
                
              if(diff)
              {
                cur->faulty_gates[cur->num_faulty_gates] = inp0->faulty_gates[fcount];   
                cur->faulty_gates[cur->num_faulty_gates].concur_out = temp;
                cur->num_faulty_gates++;              
#if (PRINT == 1)                               
                printf("+");
#endif                            
              }
            }                          
          }           
#if (PRINT == 1)                           
          printf("-First Input Gate Check\n");                  
#endif      
          for(fcount = 0; fcount < inp1->num_faulty_gates; fcount++)
          {  
            // TODO SEARCH FOR SAME FAULT to combine
            if(!inp1->faulty_gates[fcount].mark)//inp1->faulty_gates[fcount].fanout)
            {              
              evaluate_diff(cur, cur->in_val[0], inp1->faulty_gates[fcount].concur_out, temp, diff); 
              if(diff)
              {
                cur->faulty_gates[cur->num_faulty_gates] = inp1->faulty_gates[fcount];
                cur->faulty_gates[cur->num_faulty_gates].concur_out = temp;              
                cur->num_faulty_gates++;
#if (PRINT == 1)                               
                printf("+");
#endif           
              }              
            }
            inp1->faulty_gates[fcount].mark = 0;            
          }
#if (PRINT == 1)                           
          printf("-Second Input Gate Check\n");                         
#endif      
          break;
        // no input gate
        case PI:          
        case PO_GND:
        case PO_VCC:   
          break;
        default:
          assert(0);
      }    
      
      // prev fault for fault collapsing
      int prev_fault_sa0 = 0;
      int prev_fault_sa1 = 0;
      
      // FOR output
      if((cur->type >= AND && cur->type <= BUF) || cur->type == PI)
      {               
#if (PRINT == 1)    
        printf("-Output Check (%d)\n", cur->out_val);    
#endif    
        switch(cur->out_val)
        {
          case LOGIC_0:
            if(!faults_detected[ID2(cur->index, S_A_1, -1)])
            {
              prev_fault_sa0 = cur->num_faulty_gates;                
              assign_fault(cur, -1, S_A_1, 1, 0);        
            }
            break;
          case LOGIC_1:
            if(!faults_detected[ID2(cur->index, S_A_0, -1)])
            {
              prev_fault_sa1 = cur->num_faulty_gates;                                
              assign_fault(cur, -1, S_A_0, 0, 0);
            }
            break;
          case LOGIC_X:
            if(!faults_detected[ID2(cur->index, S_A_1, -1)])     
            {
              prev_fault_sa1 = cur->num_faulty_gates;                                
              assign_fault(cur, -1, S_A_1, 1, 0);
            }
            if(!faults_detected[ID2(cur->index, S_A_0, -1)])
            {
              prev_fault_sa0 = cur->num_faulty_gates;                                
              assign_fault(cur, -1, S_A_0, 0, 0); 
            }
            break;
          default:
            assert(0);
            break;
        }
      }      
      
      // Check inputs for faults                        
      if(((cur->type >= AND && cur->type <= BUF)) || cur->type == PO)
      {    
#if (PRINT == 1)                   
        printf("-First Input Check (%d)\n", cur->in_val[0]);
#endif    
        if(cur->type == PO && cur->in_val[0] == LOGIC_X)
        {
          //do nothing
        }
        else{
          switch(cur->in_val[0])        
          {
            case LOGIC_0:                       
              if(!faults_detected[ID2(cur->index, S_A_1, 0)])            
              {
                evaluate_diff(cur, LOGIC_1, cur->in_val[1], temp, diff);
                if(diff)
                {
                  if(prev_fault_sa1 && cur->type == OR || prev_fault_sa0 && cur->type == NOR)
                  {                    
                    if(!prev_fault_sa1)
                    {
                      assign_fault_to(cur, 0, S_A_1, temp, 0, prev_fault_sa1);
                    }
                    if(!prev_fault_sa0)
                    {
                      assign_fault_to(cur, 0, S_A_1, temp, 0, prev_fault_sa0);
                    }
                  }                   
                  else
                  {
                    assign_fault(cur, 0, S_A_1, temp, 0);
                  }
                }    
              }
              break;
            case LOGIC_1:
              if(!faults_detected[ID2(cur->index, S_A_1, 0)])
              {
                evaluate_diff(cur, LOGIC_0, cur->in_val[1], temp, diff);
                if(diff)
                {
                  if(prev_fault_sa1 && cur->type == NAND || prev_fault_sa0 && cur->type == AND)
                  {                    
                    if(!prev_fault_sa1)
                    {
                      assign_fault_to(cur, 0, S_A_0, temp, 0, prev_fault_sa1);
                    }
                    if(!prev_fault_sa0)
                    {
                      assign_fault_to(cur, 0, S_A_0, temp, 0, prev_fault_sa0);
                    }
                  }
                  else
                  {
                    assign_fault(cur, 0, S_A_0, temp, 0);     
                  }
                }
              }                
              break;
            case LOGIC_X:
              if(!faults_detected[ID2(cur->index, S_A_1, 0)])            
              {            
                evaluate_diff(cur, LOGIC_1, cur->in_val[1], temp, diff);
                if(diff)
                {
                  if(prev_fault_sa1 && cur->type == OR || prev_fault_sa0 && cur->type == NOR)
                  {                    
                    if(!prev_fault_sa1)
                    {
                      assign_fault_to(cur, 0, S_A_1, temp, 0, prev_fault_sa1);
                    }
                    if(!prev_fault_sa0)
                    {
                      assign_fault_to(cur, 0, S_A_1, temp, 0, prev_fault_sa0);
                    }
                  }                   
                  else
                  {                    
                    assign_fault(cur, 0, S_A_1, temp, 0); 
                  }                  
                }
              }
              if(!faults_detected[ID2(cur->index, S_A_1, 0)])
              {
                evaluate_diff(cur, LOGIC_0, cur->in_val[1], temp, diff);
                if(diff)
                {
                  if(prev_fault_sa1 && cur->type == NAND || prev_fault_sa0 && cur->type == AND)
                  {                    
                    if(!prev_fault_sa1)
                    {
                      assign_fault_to(cur, 0, S_A_0, temp, 0, prev_fault_sa1);
                    }
                    if(!prev_fault_sa0)
                    {
                      assign_fault_to(cur, 0, S_A_0, temp, 0, prev_fault_sa0);
                    }
                  }
                  else
                  {                    
                    assign_fault(cur, 0, S_A_0, temp, 0); 
                  }
                } 
              }
              break;
            default:
              assert(0);
              break;            
          }
        }
      }  

      // FOR 2 inputs
      if(cur->type >= AND && cur->type <= NOR)
      {  
#if (PRINT == 1)  
        printf("-Second Input Check (%d)\n", cur->in_val[1]);
#endif    
        switch(cur->in_val[1])
        {
          case LOGIC_0:
            if(!faults_detected[ID2(cur->index, S_A_1, 1)])
            {         
              evaluate_diff(cur, cur->in_val[0], LOGIC_1, temp, diff);
              if(diff)
              {
                if(prev_fault_sa1 && cur->type == OR || prev_fault_sa0 && cur->type == NOR)
                {                    
                  if(!prev_fault_sa1)
                  {
                    assign_fault_to(cur, 1, S_A_1, temp, 0, prev_fault_sa1);
                  }
                  if(!prev_fault_sa0)
                  {
                    assign_fault_to(cur, 1, S_A_1, temp, 0, prev_fault_sa0);
                  }
                }                   
                else
                {                  
                  assign_fault(cur, 1, S_A_1, temp, 0);
                }
              } 
            }
            break;
          case LOGIC_1:
            if(!faults_detected[ID2(cur->index, S_A_0, 1)])                    
            {    
              evaluate_diff(cur, cur->in_val[0], LOGIC_0, temp, diff);
              if(diff)
              {
                if(prev_fault_sa1 && cur->type == NAND || prev_fault_sa0 && cur->type == AND)
                {                    
                  if(!prev_fault_sa1)
                  {
                    assign_fault_to(cur, 1, S_A_0, temp, 0, prev_fault_sa1);
                  }
                  if(!prev_fault_sa0)
                  {
                    assign_fault_to(cur, 1, S_A_0, temp, 0, prev_fault_sa0);
                  }
                }
                else
                {                       
                  assign_fault(cur, 1, S_A_0, temp, 0);    
                }
              }
            }              
            break;
          case LOGIC_X:
            if(!faults_detected[ID2(cur->index, S_A_1, 1)])
            {
              evaluate_diff(cur, cur->in_val[0], LOGIC_1, temp, diff);
              if(diff)
              {
                if(prev_fault_sa1 && cur->type == OR || prev_fault_sa0 && cur->type == NOR)
                {                    
                  if(!prev_fault_sa1)
                  {
                    assign_fault_to(cur, 1, S_A_1, temp, 0, prev_fault_sa1);
                  }
                  if(!prev_fault_sa0)
                  {
                    assign_fault_to(cur, 1, S_A_1, temp, 0, prev_fault_sa0);
                  }
                }                   
                else
                {                     
                  assign_fault(cur, 1, S_A_1, temp, 0);
                }
              }          
            }
            if(!faults_detected[ID2(cur->index, S_A_0, 1)])                    
            {                
              evaluate_diff(cur, cur->in_val[0], LOGIC_0, temp, diff);
              if(diff)
              {
                if(prev_fault_sa1 && cur->type == NAND || prev_fault_sa0 && cur->type == AND)
                {                    
                  if(!prev_fault_sa1)
                  {
                    assign_fault_to(cur, 1, S_A_0, temp, 0, prev_fault_sa1);
                  }
                  if(!prev_fault_sa0)
                  {
                    assign_fault_to(cur, 1, S_A_0, temp, 0, prev_fault_sa0);
                  }
                }
                else
                {                       
                  assign_fault(cur, 1, S_A_0, temp, 0);                 
                }
              }
            }
            break;
          default:
            assert(0);
            break;            
        }      
      }      
    }
    
#if (PRINT ==1)    
    // check fault list
    int j;
    for(i = 0; i < ckt->ngates; i++)
    {
      gate_t* cur = &ckt->gate[i];
      printf("gate: %d (num_faulty: %d)\n", i, cur->num_faulty_gates);
      for(j = 0; j < cur->num_faulty_gates; j++)
      {
        printf("%d.%d/%d\n", cur->faulty_gates[j].gate_index, cur->faulty_gates[j].input_index, cur->faulty_gates[j].type);
      }
    }    
#endif    
    
    // check all 
    for(o = 0; o < ckt_npo; o++)
    {
      gate_t* out = &(ckt->gate[ckt->po[o]]);
      for(f = out->num_faulty_gates - 1; f >= 0; f--)
      {
          fault_list_t* cur_fault = undetected_flist;
          fault_list_t* prev_fault = NULL;
          while(cur_fault != NULL)
          {
            if(cur_fault->gate_index == out->faulty_gates[f].gate_index && cur_fault->input_index == out->faulty_gates[f].input_index && cur_fault->type == out->faulty_gates[f].type)
            {              
              if(prev_fault == NULL){
                undetected_flist = cur_fault->next;                
              }
              else{
                prev_fault->next = cur_fault->next;                         
              }              
              faults_detected[ID1(out->faulty_gates[f])] = 1; // fault dropping             
            }   
            else
            {
              prev_fault = cur_fault;
            }
            cur_fault = cur_fault->next;
          }
      }
      if(undetected_flist == NULL)
      {
        done == 1;
        break;
      }      
    }
    
  }
  }
  
  return(undetected_flist);
}
