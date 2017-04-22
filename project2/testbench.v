//`timescale 1ns/10ps
`define NULL 0
`define EOF 32'hFFFF_FFFF
`define FAULTFREE 1
`define BISTTEST 2

module TOP_ctl ;
   
 integer c, r, file_in, file_out;
 integer mode;
 integer detected_faults;
 integer faults;

   //Input registers
 reg [34:0] _pi;       //PI
 reg [0:0] _clk;       //clock
 reg [0:0] _rst;       //Reset
 reg [0:0] _bistmode;  //Bistmode

   //Output wires
 wire [0:0] _bistpass; //Bistpass
 wire [0:0] _bistdone; //Bistdone
 wire [48:0] _cut_po;  //PO

 parameter cycle = 100; //cycle time

   //Wires for external interface
wire pin_1_, pin_2_, pin_3_, pin_4_, pin_5_, pin_6_,
   pin_7_, pin_8_, pin_9_, pin_10_, pin_11_, pin_12_, pin_13_,
   pin_14_, pin_15_, pin_16_, pin_17_, pin_18_, pin_19_, pin_20_,
   pin_21_, pin_22_, pin_23_, pin_24_, pin_25_, pin_26_, pin_27_,
   pin_28_, pin_29_, pin_30_, pin_31_, pin_32_, pin_33_, pin_34_,
   pin_35_, pin_36_, pin_37_, pin_38_, pin_39_, pin_40_, pin_41_,
   pin_42_, pin_43_, pin_44_, pin_45_, pin_46_, pin_47_, pin_48_,
   pin_49_, pin_50_, pin_51_, pin_52_, pin_53_, pin_54_, pin_55_,
   pin_56_, pin_57_, pin_58_, pin_59_, pin_60_, pin_61_, pin_62_,
   pin_63_, pin_64_, pin_65_, pin_66_, pin_67_, pin_68_, pin_69_,
   pin_70_, pin_71_, pin_72_, pin_73_, pin_74_, pin_75_, pin_76_,
   pin_77_, pin_78_, pin_79_, pin_80_, pin_81_, pin_82_, pin_83_,
   pin_84_, pin_85_, pin_86_, pin_87_, pin_88_, pin_89_;


assign _bistdone = {pin_1_};
assign _bistpass = {pin_2_};
assign {pin_3_} = _bistmode;
assign {pin_4_} = _clk;
assign {pin_5_} = _rst;
assign {pin_6_, pin_7_,
   pin_8_, pin_9_, pin_10_, pin_11_, pin_12_, pin_13_, pin_14_,
   pin_15_, pin_16_, pin_17_, pin_18_, pin_19_, pin_20_, pin_21_,
   pin_22_, pin_23_, pin_24_, pin_25_, pin_26_, pin_27_, pin_28_,
   pin_29_, pin_30_, pin_31_, pin_32_, pin_33_, pin_34_, pin_35_,
   pin_36_, pin_37_, pin_38_, pin_39_, pin_40_ } = _pi;

assign _cut_po = { pin_41_, pin_42_,
   pin_43_, pin_44_, pin_45_, pin_46_, pin_47_, pin_48_, pin_49_,
   pin_50_, pin_51_, pin_52_, pin_53_, pin_54_, pin_55_, pin_56_,
   pin_57_, pin_58_, pin_59_, pin_60_, pin_61_, pin_62_, pin_63_,
   pin_64_, pin_65_, pin_66_, pin_67_, pin_68_, pin_69_, pin_70_,
   pin_71_, pin_72_, pin_73_, pin_74_, pin_75_, pin_76_, pin_77_,
   pin_78_, pin_79_, pin_80_, pin_81_, pin_82_, pin_83_, pin_84_,
   pin_85_, pin_86_, pin_87_, pin_88_, pin_89_ };


   // instantiate chip here
   chip testchip(
   	.clk(pin_4_),
	.rst(pin_5_),
	.pi( {pin_6_, pin_7_,
   pin_8_, pin_9_, pin_10_, pin_11_, pin_12_, pin_13_, pin_14_,
   pin_15_, pin_16_, pin_17_, pin_18_, pin_19_, pin_20_, pin_21_,
   pin_22_, pin_23_, pin_24_, pin_25_, pin_26_, pin_27_, pin_28_,
   pin_29_, pin_30_, pin_31_, pin_32_, pin_33_, pin_34_, pin_35_,
   pin_36_, pin_37_, pin_38_, pin_39_, pin_40_ } ),
	.po( { pin_41_, pin_42_,
   pin_43_, pin_44_, pin_45_, pin_46_, pin_47_, pin_48_, pin_49_,
   pin_50_, pin_51_, pin_52_, pin_53_, pin_54_, pin_55_, pin_56_,
   pin_57_, pin_58_, pin_59_, pin_60_, pin_61_, pin_62_, pin_63_,
   pin_64_, pin_65_, pin_66_, pin_67_, pin_68_, pin_69_, pin_70_,
   pin_71_, pin_72_, pin_73_, pin_74_, pin_75_, pin_76_, pin_77_,
   pin_78_, pin_79_, pin_80_, pin_81_, pin_82_, pin_83_, pin_84_,
   pin_85_, pin_86_, pin_87_, pin_88_, pin_89_ } ),
   	.bistmode( pin_3_ ),
	.bistdone( pin_1_ ),
	.bistpass( pin_2_ )
   );
   	
   always #(cycle / 2) _clk = ~_clk; //clock generator

//For fault free simulation: display PI and PO values
always @(posedge _clk) begin
	#5;
	if ( mode == `FAULTFREE ) begin
		writepi;
		writepo;
		$write( "\n" );
	end
end

//Fault free simulation task
task faultfree;
begin : file_block
      $display("Functional Simulation in system mode\n");
         
      mode = `FAULTFREE; //set mode

      file_in  = $fopen("vectors");            //Open input file
      file_out = $fopen("golden_output");       //Open output file 
      
      if(`NULL==file_in || `NULL==file_out)
	 disable file_block; //exit

      @(negedge _clk) begin                     //Reset=1, disable BIST
          _rst = 1;
	  _bistmode = 0;
          #(cycle-1) _rst = 0;                  //Reset=0
      end

      //Apply inputs, write outputs
      while(!$feof(file_in)) begin

          @(negedge _clk) begin
              r=$fscanf(file_in, "%b\n", _pi);     //Apply vector
	  end

          #(cycle-1);
              $fwrite(file_out, "%b\n", _cut_po);    //Write golden output
      end // while (c!= `EOF)

      $fclose(file_out);         //Close output file for write
      $fclose(file_in);     //Close input file for read

end
endtask

//BIST task
task dobist;
begin
   $display("Starting BIST run");
   
   mode = `BISTTEST;            //set mode

   @(negedge _clk) begin        //Reset=1, enable BIST
   	_rst = 1;
   	_bistmode = 1;
   end

   #(cycle-1) _rst = 0;

   @(_bistdone == 1) begin      //When Bist done and bistpass status
      $write( "Completed Bist...  BIST Passed: %b\n", _bistpass );
      if(_bistpass == 0)
	 detected_faults = detected_faults+1;
   end
end
endtask

initial begin
   $write("PI                                    PO\n");
   _clk = 1'b0;
   detected_faults = 0;
   faults = 0;

//   faultfree;                            //Fault free simulation

   //Fault Injection
   $write( "\nInjecting No Fault into CUT:  Fault-Free BIST Simulation\n" );
   dobist;

   force testchip.circuit.II282 = 1;
   $write( "\nInjecting Fault into CUT:  testchip.circuit.II282 = 1\n" );
   faults = faults+1;
   dobist;
   release testchip.circuit.II282;

   
   force testchip.circuit.n482gat = 0;
   $write( "\nInjecting Fault into CUT:  testchip.circuit.n482gat = 0\n" );
   faults = faults+1;
   dobist;
   release testchip.circuit.n482gat;

   force testchip.circuit.n226gat = 1;
   $write( "\nInjecting Fault into CUT:  testchip.circuit.n226gat = 1\n" );
   faults = faults+1;
   dobist;
   release testchip.circuit.n226gat;

   force testchip.circuit.n1275gat = 0;
   $write( "\nInjecting Fault into CUT:  testchip.circuit.n1275gat = 0\n" );
   faults = faults+1;
   dobist;
   release testchip.circuit.n1275gat;

   force testchip.circuit.n1582gat = 1;
   $write( "\nInjecting Fault into CUT:  testchip.circuit.n1582gat = 1\n" );
   faults = faults+1;
   dobist;
   release testchip.circuit.n1582gat;

   force testchip.circuit.II4020 = 0;
   $write( "\nInjecting Fault into CUT:  testchip.circuit.II4020 = 0\n " );
   faults = faults+1;
   dobist;
   release testchip.circuit.II4020;

   //Fault Injection
   $write( "\nInjecting No Fault into CUT:  Fault-Free BIST Simulation\n" );
   dobist;   
   
   force testchip.circuit.n745gat = 1;
   $write( "\nInjecting Fault into CUT:  testchip.circuit.n745gat = 1\n " );
   faults = faults+1;
   dobist;
   release testchip.circuit.n745gat;

   //Fault Injection
   $write( "\nInjecting No Fault into CUT:  Fault-Free BIST Simulation\n" );
   dobist;   
   
   force testchip.circuit.n1794gat = 0;
   $write( "\nInjecting Fault into CUT:  testchip.circuit.n1794gat = 0\n " );
   faults = faults+1;
   dobist;
   release testchip.circuit.n1794gat;

   //Fault Injection
   $write( "\nInjecting No Fault into CUT:  Fault-Free BIST Simulation\n" );
   dobist;   
   
   force testchip.circuit.II4623 = 1;
   $write( "\nInjecting Fault into CUT:  testchip.circuit.II4623 = 1\n " );
   faults = faults+1;
   dobist;
   release testchip.circuit.II4623;

   force testchip.circuit.II4759 = 0;
   $write( "\nInjecting Fault into CUT:  testchip.circuit.II4759 = 0\n " );
   faults = faults+1;
   dobist;
   release testchip.circuit.II4759;

   //Fault Injection
   $write( "\nInjecting No Fault into CUT:  Fault-Free BIST Simulation\n" );
   dobist;
   
   $write("\nInjected %d faults\n", faults);
   $write("Detected %d faults\n", detected_faults);

   $finish;
end // initial begin

   
task writepi;
begin
	$write( "%b ", 
	   {testchip.circuit.n3065gat,testchip.circuit.n3066gat,testchip.circuit.n3067gat,testchip.circuit.n3068gat,testchip.circuit.n3069gat,
	   testchip.circuit.n3070gat,testchip.circuit.n3071gat,testchip.circuit.n3072gat,
	   testchip.circuit.n3073gat,testchip.circuit.n3074gat,testchip.circuit.n3075gat,testchip.circuit.n3076gat,testchip.circuit.n3077gat,testchip.circuit.n3078gat,testchip.circuit.n3079gat,testchip.circuit.n3080gat,
	   testchip.circuit.n3081gat,testchip.circuit.n3082gat,testchip.circuit.n3083gat,testchip.circuit.n3084gat,testchip.circuit.n3085gat,testchip.circuit.n3086gat,testchip.circuit.n3087gat,testchip.circuit.n3088gat,
	   testchip.circuit.n3089gat,testchip.circuit.n3090gat,testchip.circuit.n3091gat,testchip.circuit.n3092gat,testchip.circuit.n3093gat,testchip.circuit.n3094gat,testchip.circuit.n3095gat,testchip.circuit.n3097gat,
	   testchip.circuit.n3098gat,testchip.circuit.n3099gat,testchip.circuit.n3100gat} );

end
endtask

task writepo;
begin
	$write( "%b ", {
		testchip.circuit.n3104gat,testchip.circuit.n3105gat,testchip.circuit.n3106gat,testchip.circuit.n3107gat,testchip.circuit.n3108gat,testchip.circuit.n3109gat,testchip.circuit.n3110gat,
		testchip.circuit.n3111gat,testchip.circuit.n3112gat,testchip.circuit.n3113gat,testchip.circuit.n3114gat,testchip.circuit.n3115gat,testchip.circuit.n3116gat,testchip.circuit.n3117gat,testchip.circuit.n3118gat,testchip.circuit.n3119gat,
		testchip.circuit.n3120gat,testchip.circuit.n3121gat,testchip.circuit.n3122gat,testchip.circuit.n3123gat,testchip.circuit.n3124gat,testchip.circuit.n3125gat,testchip.circuit.n3126gat,testchip.circuit.n3127gat,
		testchip.circuit.n3128gat,testchip.circuit.n3129gat,testchip.circuit.n3130gat,testchip.circuit.n3131gat,testchip.circuit.n3132gat,testchip.circuit.n3133gat,testchip.circuit.n3134gat,testchip.circuit.n3135gat,
		testchip.circuit.n3136gat,testchip.circuit.n3137gat,testchip.circuit.n3138gat,testchip.circuit.n3139gat,testchip.circuit.n3140gat,testchip.circuit.n3141gat,testchip.circuit.n3142gat,testchip.circuit.n3143gat,
		testchip.circuit.n3144gat,testchip.circuit.n3145gat,testchip.circuit.n3146gat,testchip.circuit.n3147gat,testchip.circuit.n3148gat,testchip.circuit.n3149gat,testchip.circuit.n3150gat,testchip.circuit.n3151gat,
		testchip.circuit.n3152gat} );

end
endtask

endmodule // TOP_ctl


