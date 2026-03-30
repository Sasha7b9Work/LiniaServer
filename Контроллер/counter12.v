  

module counter12 (clk, start, count, line, ovl, flag, dp_flg);
 // integer count; 
  input  clk;
  input start;
  input line;
  output  count;
  output ovl;
  output flag;
  output dp_flg; 
 reg [7:0] count;  
 reg  ovl; 
 reg flag;
 reg dp_flg;
 	

  always @ (posedge clk or posedge start)
		if(start)
               begin
			flag <= 0;
			count <= 8'd0;
			              end 
                 else
				
		        if (flag)// "1"
			      begin
				count <= 8'd0;  // count=0
						       end 
		             else 
               if (!line || dp_flg)// line=0 and  dp_flg=1
			        begin
				count <= count + 1'b1; // 
				if (count==8'd4)// end of delay
					begin
						ovl <= 1'b1;//start pulse
						dp_flg <= 1'b1; // dp_flg="1"
					       end
				             else
                       if (count==8'd10)//control end pulse
					 begin 
						ovl <= 1'b0;//end pulse
						             end
                      if (count==8'd16)//control end count
                      begin 
						flag<= 1'b1;
					             end
    	                               end
 
                       endmodule


