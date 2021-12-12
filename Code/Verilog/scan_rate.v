/*Description: Prescales (divides) down the 
system clock, 50 MHz (20ns) based on prescaler
value. Outputs 1 high pulse of a clk period 
signal. 

Parameters: 50 MHz clock (20ns)
	    prescaler = 8
*/

module scan_rate #(parameter prescaler = 8)(
input clk,
input reset_bar,
output q);

//parameter prescaler = 8;
integer count = prescaler - 1;
reg qtemp = 1'b0;

always @(posedge clk) begin
if(!reset_bar) begin	//if reset (active low), set reset values 
	count <= prescaler - 1;//reset count to be prescaler - 1
	qtemp <= 1'b0;	//reset qtemp to be low
end
else
if (count == 0) begin	//if count is 0, then set the 1 high clk period pulse
	qtemp <= 1'b1;	
	count <= prescaler - 1;//And also reset the count
end
else begin	//if count is not 0, then keep counting (decrementing)
	qtemp <= 1'b0;//qtemp is set back to 0
	count <= count - 1;//decrement count
end
end

assign q = qtemp;
endmodule 
