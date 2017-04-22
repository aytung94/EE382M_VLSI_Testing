module LFSR_hardware(lfsr, nextLFSR, inputLFSR, rst);

  output reg [15:0] lfsr;
  input rst;
  input nextLFSR;
  input inputLFSR;  
  
  initial 
  begin
    lfsr <= 16'h0001;
  end

  always @ (negedge nextLFSR or posedge rst)
  begin
    if(rst)
    begin
      lfsr <= 16'h0001;
    end   
    else
    begin
      lfsr[15:0] <= {lfsr[14:0], lfsr[15] ^ lfsr[4] ^ lfsr[3] ^ lfsr[2] ^ inputLFSR};
    end
  end
endmodule

module SISR_hardware(sisr, nextSISR, inputSISR, clk, rst);

  output reg [15:0] sisr;
  input clk;
  input rst;
  input nextSISR;
  input inputSISR;  
  
  initial 
  begin
    sisr <= 16'h0000;
  end

  always @ (negedge clk or posedge rst)
  begin
    if(rst)
    begin
      sisr <= 16'h0000;
    end   
    else
    begin
      if(nextSISR)
      begin
        sisr[15:0] <= {sisr[14:0], sisr[15] ^ sisr[4] ^ sisr[3] ^ sisr[2] ^ inputSISR};
      end
      else
      begin
        // do nothing
      end
    end
  end
endmodule

module bist_hardware(clk,rst,bistmode,bistdone,bistpass,cut_scanmode,
                     cut_sdi,cut_sdo);
  input          clk;         
  input          rst;         
  input          bistmode;    
  output         bistdone;    
  output         bistpass;
  output         cut_scanmode;
  output         cut_sdi;
  input          cut_sdo;
  
// Add your code here
  reg test;
  reg [2:0] state;
  reg [2:0] nextState;
  reg [9:0] counter; 
  reg [9:0] nextCounter;
  reg [10:0] pattern;
  reg [10:0] nextPattern;
  reg _bistdone;
  reg _bistpass;
  reg _cut_scanmode;
  reg _cut_sdi;
  
  // LFSR
  wire [15:0] LFSR;  
  reg resetLFSR;
  LFSR_hardware lfsr_hw(LFSR, clk, 1'b0, resetLFSR);
  
  // SISR
  wire [15:0] SISR;
  reg nextSISR;  
  reg resetSISR;  
  SISR_hardware sisr_hw(SISR, nextSISR, cut_sdo, clk, resetSISR);
  
  // assign reg to output
  assign bistdone = _bistdone;
  assign bistpass = _bistpass;
  assign cut_scanmode = _cut_scanmode;
  assign cut_sdi = _cut_sdi;
  
  initial 
  begin
    test <= 0;
    state <= 6;
    nextState <= 0;
    counter <= 0;
    nextCounter <= 0;
    pattern <= 0;
    nextPattern <= 0;
    _bistdone <= 0;
    _bistpass <= 0;
    _cut_scanmode <= 0;
    _cut_sdi <= 0;    
    resetLFSR <= 1;
    nextSISR <= 0;
    resetSISR <= 1;    
  end
  
  always @ (*)
  begin  
    nextState <= nextState;
    nextCounter <= nextCounter;
    nextPattern <= nextPattern;    
    _bistpass <= _bistpass;
    _bistdone <= _bistdone;  
    _cut_scanmode <= _cut_scanmode;
    _cut_sdi <= _cut_sdi;
    resetLFSR <= resetLFSR;    
    nextSISR <= nextSISR;    
    resetSISR <= resetSISR;
    case (state)
      0: begin
         nextState <= 1;
         nextCounter <= 0;
         nextPattern <= 0;
         _bistpass <= 0;
         _bistdone <= 0;
         _cut_scanmode <= 0;  
         _cut_sdi <= 0;                  
         resetLFSR <= 0; // start lfsr
         nextSISR <= 0;         
         resetSISR <= 0;         
         end
      1: begin // first shift in
        if(counter < 227)
        begin
          nextState <= 1;        
        end
        else // (counter == 227)
        begin
          nextState <= 2;
        end      
        nextCounter <= counter + 1;        
        _cut_sdi <= LFSR[0]; // take tap 0
        _cut_scanmode <= 1; // begin first shift in
        end
      2: begin // clock CUT       
        nextState <= 3;        
        nextCounter <= 0;
        _cut_scanmode <= 0;        
        end
      3: begin // shift in/out
        if(counter < 227)
        begin
          nextState <= 3;
        end
        else // (counter == 227)
        begin 	 
          nextState <= 4;
        end
        nextCounter <= counter + 1;	
        nextSISR <= 1;
        _cut_sdi <= LFSR[0];	
        _cut_scanmode <= 1;
        end
      4: begin // clock CUT
        if(pattern < 2000)
        begin
          nextState <= 3;
        end
        else
        begin // (pattern == 2000)
          nextState <= 5;
        end	
        nextCounter <= 0;
        nextPattern <= pattern + 1;        
        nextSISR <= 0;	
        _cut_scanmode <= 0;
        end
      5: begin // complete       
        nextState <= 6; 
        nextCounter <= 0;
        nextPattern <= 0;     
        $write( "SISR: 0x%h vs. 0xaa80\n", SISR);          
        _bistdone <= 1;      
        _cut_scanmode <= 0;                    
        if(SISR == 16'hAA80) // SISR == hard coded value
        begin        
          _bistpass <= 1;          
        end
        else
        begin
          _bistpass <= 0;   
        end        
        end
      6: begin // wait to restart
        // do nothing	
        nextState <= 0;
        resetLFSR <= 1;
        resetSISR <= 1;   
        //_bistdone <= 0; can let it be since we fix test
        _cut_scanmode <= 0;              
        end
      default:begin
	    // do nothing
	    end
    endcase
  end
  
  always @ (posedge rst or posedge bistdone)
  begin
    if(rst == 1)
    begin
      if(bistmode == 1)      
      begin
        test = 1;
      end
      else
      begin
        test = 0;
      end
      nextState = 6;      
      _bistdone = 0;
    end
    else
    begin
      // do nothing
      if(bistdone == 1)
      begin
        test = 0;
/*        if(bistpass == 1)
        begin
          $write( "SISR: 0x%h === 0xaa80\n", SISR);        
        end
        else
        begin
          $write( "SISR: 0x%h =\= 0xaa80\n", SISR);        
        end*/
      end
    end
  end

  always @ (negedge clk)
  begin
    if(test == 1)
    begin
      state <= nextState;
      counter <= nextCounter;
      pattern <= nextPattern;
    end
    else
    begin
      state <= 6; 
      counter <= 0;
      pattern <= 0;     
    end
  end

endmodule




module chip(clk,rst,pi,po,bistmode,bistdone,bistpass);
  input          clk;
  input          rst;
  input  [34:0]  pi;
  output [48:0]  po;
  input          bistmode;
  output         bistdone;
  output         bistpass;

  wire           cut_scanmode,cut_sdi,cut_sdo;

  reg x;
  wire w_x;
  assign w_x = x;

  scan_cut circuit(bistmode,cut_scanmode,cut_sdi,cut_sdo,clk,rst,
         pi[0],pi[1],pi[2],pi[3],pi[4],pi[5],pi[6],pi[7],pi[8],pi[9],
         pi[10],pi[11],pi[12],pi[13],pi[14],pi[15],pi[16],pi[17],pi[18],pi[19],
         pi[20],pi[21],pi[22],pi[23],pi[24],pi[25],pi[26],pi[27],pi[28],pi[29],
         pi[30],pi[31],pi[32],pi[33],pi[34],
         po[0],po[1],po[2],po[3],po[4],po[5],po[6],po[7],po[8],po[9],
         po[10],po[11],po[12],po[13],po[14],po[15],po[16],po[17],po[18],po[19],
         po[20],po[21],po[22],po[23],po[24],po[25],po[26],po[27],po[28],po[29],
         po[30],po[31],po[32],po[33],po[34],po[35],po[36],po[37],po[38],po[39],
         po[40],po[41],po[42],po[43],po[44],po[45],po[46],po[47],po[48]);
  bist_hardware bist( clk,rst,bistmode,bistdone,bistpass,cut_scanmode,
                     cut_sdi,cut_sdo);

endmodule
