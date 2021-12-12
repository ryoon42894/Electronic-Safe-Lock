/*Description: Receives the row array that
has a single 0 in it and changes the 1's in 
the row array into high impedance (Z's).
*/
module row_buffer(
input [3:0] row_in,
output [3:0] row_driver);

integer i;
reg [3:0] row_driver_temp;

always @(row_in) begin
for (i=0;i<4;i=i+1) begin	//go through each element of row array
	if (row_in[i] == 1'b1) begin//to detect the 1's, if it is 1
		row_driver_temp[i] <= 1'bZ;//then set it to Z
	end
	else begin		//else, set it to 0
		row_driver_temp[i] <= 1'b0;
	end
end
end
assign row_driver = row_driver_temp;
endmodule 
