module LFSR_hardware(lfsr, nextLFSR, inputLFSR, rst);

  output reg [15:0] lfsr;
  input rst;
  input nextLFSR;
  input inputLFSR;  
  
  initial 
  begin
    lfsr <= 16'h0001;
  end

  always @ (posedge nextLFSR or posedge rst)
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

module SIG_hardware(sig, nextSIG, inputSIG, clk, rst);

  output reg [15:0] sig;
  input clk;
  input rst;
  input nextSIG;
  input inputSIG;  
  
  initial 
  begin
    sig <= 16'h0000;
  end

  always @ (posedge clk or posedge rst)
  begin
    if(rst)
    begin
      sig <= 16'h0000;
    end   
    else
    begin
      if(nextSIG)
      begin
        sig[15:0] <= {sig[14:0], sig[15] ^ sig[4] ^ sig[3] ^ sig[2] ^ inputSIG};
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
  input          clk;          // used con
  input          rst;          // used con
  input          bistmode;     // used
  output reg        bistdone;     // used 
  output reg        bistpass;
  output reg        cut_scanmode;
  output reg        cut_sdi;
  input          cut_sdo;
  
// Add your code here
  reg [2:0] state;
  reg [2:0] nextState;
  reg [9:0] counter; 
  reg [9:0] nextCounter;
  reg [10:0] pattern;
  reg [10:0] nextPattern;

  wire [15:0] LFSR;  
  reg nextLFSR;
  reg resetLFSR;
  LFSR_hardware lfsr_hw(LFSR, nextLFSR, 0, resetLFSR);
  
  wire [15:0] SIG;
  reg nextSIG;  
  reg resetSIG;  
  SIG_hardware sig_hw(SIG, nextSIG, cut_sdo, clk, resetSIG);
  
  reg test;
  
  initial 
  begin
    test <= 0;
    state <= 6;
    nextState <= 0;
    bistdone <= 0;
    bistpass <= 0;
    resetLFSR <= 1;
    resetSIG <= 1;
  end
  
  always @ (*)
  begin  
    nextState <= nextState;
    nextCounter <= nextCounter;
    nextPattern <= nextPattern;    
    nextLFSR <= nextLFSR;
    nextSIG <= nextSIG;
    bistpass <= bistpass;
    bistdone <= bistdone;  
    cut_scanmode <= cut_scanmode;
    cut_sdi <= cut_sdi;
    case (state)
      0: begin
           nextState <= 1;
           nextCounter <= 0;
           nextPattern <= 0;
           nextLFSR <= 0;  
           nextSIG <= 0;
           bistpass <= 0;
           bistdone <= 0;
           resetLFSR <= 0;
           resetSIG <= 0;
         end
      1: begin
        if(counter < 227)
        begin
          nextState <= 1;        
        end
        else // (counter == 227)
        begin
          nextState <= 2;
        end      
        nextCounter <= counter + 1;        
        cut_sdi <= LFSR[counter % 16];
        cut_scanmode <= 1;
        end
      2: begin        
        nextState <= 3;        
        nextCounter <= 0;
        nextLFSR <= 1;
        cut_scanmode <= 0;        
        end
      3: begin
        if(counter < 227)
        begin
          nextState <= 3;
        end
        else // (counter == 227)
        begin 	 
          nextState <= 4;
        end
        nextCounter <= counter + 1;	
        nextLFSR <= 0;        
        nextSIG <= 1;
        cut_sdi <= LFSR[counter % 16];	
        cut_scanmode <= 1;
        end
      4: begin
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
        nextLFSR <= 1;
        nextSIG <= 0;	
        cut_scanmode <= 0;
        end
      5: begin       
        nextState <= 6; 
        nextCounter <= 0;
        nextPattern <= 0;     
        bistdone <= 1;        
        if(SIG == 16'h1d15) // MISR == hard coded value
        begin
          bistpass <= 1;
        end
        else
        begin
          bistpass <= 0;   
        end
        end
      6: begin
          // do nothing	
          nextState <= 0;
          resetLFSR <= 1;
          resetSIG <= 1;   
          bistdone <= 0;
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
    end
    else
    begin
      // do nothing
      if(bistdone == 1)
      begin
        test = 0;
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