/*Description: Detects which button is pressed 
(active low) on a 4x4 keypad. Utilizes a walking 
0 method, a 0 is individually walked to each row. 
The columns on the row with the 0 will be checked; 
if a column  reads a 0, then a column on that row 
has been pressed and the value/location is processed 
and outputted on key_code. If all columns in the row 
read 1s, then the next row is walked with the 0, and 
it's column is scanned. It will continue to scan the 
columns in the row that is walked with a 0.
*/
module key_encoder(
input [3:0] row_in,
input [3:0] col_in,
output kp_bar,
output [3:0] key_code);

reg [3:0] key_code_temp;
reg kp_bar_temp = 1'b1;
integer i, j;

always @(row_in,col_in) begin
for (i=0;i<4;i=i+1)begin 	    //nested loop, 1st loop will go through rows 0-3
	if (row_in[i] == 1'b0) begin//if the row in the index is 0, scan the columns in the row
		for(j=0;j<4;j=j+1) begin //nested loop, 2nd loop will go through columns 0-3
			if (col_in == 4'b1111) begin//if all columns are high, then no key has been pressed
				kp_bar_temp = 1'b1;//set kp_bar (active low) high, i.e. not pressed
				//j = 4;
			end
			else if (col_in[j] == 1'b0) begin//if a column is high, then that column in the row being scanned is pressed 
				key_code_temp = (i * 4) + j;//calculates the location/value of the key pressed
				kp_bar_temp = 1'b0;	   //activate kp_bar
			end
		end
	end
	/*else if (col_in == 4'b1111) begin
		kp_bar_temp = 1'b1;
		key_code_temp = 4'b0000;
	end*/
end
end
assign key_code = key_code_temp;
assign kp_bar = kp_bar_temp;
endmodule
