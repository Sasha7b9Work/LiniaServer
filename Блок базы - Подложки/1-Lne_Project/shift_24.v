module shift_24  (id,iclk,eff,enb);
  
    input id;
    input enb; 
    wire enb;
    input iclk;
    output [23:0] eff;
    reg [23:0] dff;
    reg [23:0] eff;
 
   
    always @(posedge iclk or  posedge enb) begin
               if(enb) begin
                eff <= dff;
                  end
                  
					else begin
                   dff <= {dff[22:0], id};
                  end
                      end
 endmodule
 
