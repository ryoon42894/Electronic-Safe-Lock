/*Description: Implements the walking 0 method.
A row will be 0 while the others will be 1s. If
the row_scanner is enabled from both enables,
then the 0 will be moved to the next row, and the
others will be 1s. This method is utilized using
a STATE machine. 
*/
module row_scanner(
input rst_bar,
input clk,
input en1,
input en2,
output [3:0]qout);

reg [3:0] qout_temp;
integer STATE = 0;

always @(posedge clk) begin	//determines the STATE
if(!rst_bar) begin		//if resetted (active low) then reset
	//qout_temp <= 4'b0111;	//STATE back to 0
	STATE <= 0;
end
else if (en1 && en2) begin	//if both enables are activated, then walk the 0 to the next row
if (STATE < 3) begin		//if STATE is less than 3, then increment STATE to the next STATE
	STATE <= STATE + 1;
end
else begin			//if not, then reset STATE back to 0
	STATE <= 0;
end
end
end

always @(en1,en2) begin		//combinational output process
case (STATE) 			//case dependent on STATE
	0 : qout_temp = 4'b1110;//STATE 0 has the 1st row as 0
	1 : qout_temp = 4'b1101;//STATE 1 has the 2nd row as 0
	2 : qout_temp = 4'b1011;//STATE 2 has the 3rd row as 0
	3 : qout_temp = 4'b0111;//STATE 3 has the 4th row as 0
	default : qout_temp = 4'b1110;//default has the 1st row as 0
endcase
end
assign qout = qout_temp;
endmodule 
