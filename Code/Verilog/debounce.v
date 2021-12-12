/*Description: Debounces the key press from
key_encoder. Following the principle of a
one-shot circuit, if a key is pressed
the debounce will stretch (delay) the received 
active low kp_bar signal from key_encoder. It will 
stretch both the 'make' and 'break' bounce DURATION 
long. The stretched or debounced kp_bar
signal will be qbar which will go into and act
as one of the enables for row_scanner. Only within 
the 'make' bounce, the term signal will be set 
high for 1 clk period. Term acts as an enable to 
the spi_interface which tells spi_interface to store 
the key_code value and let the spi master know
that a key_code is ready to be outputted. Once 
debounce time is finished, 
4 processes: 1 process to generate a 1 clk
delayed a signal, 1 process to generate an
a signal on a negative edge of clk, 1 process to handle
the count of the debounce time, 1 process to
handle the outputs (term, qbar) which are based
on count and a delayed. 

Parameter: 50 MHz clock (20ns)
	   DURATION: 1000000 clock cycles (20ms)
*/

module debounce #(parameter DURATION = 1000000)(
input clk,
input rst_bar,
input a,
output qbar,
output term);

reg a_delay = 1'b0;
reg a_nedg = 1'b0;
reg qbar_temp;
reg term_temp;
integer count = 0;

always @(posedge clk) begin //generate a_delay
if (!rst_bar || a == 1'b0) begin//if on resetted (active low) or a is low
	a_delay <= 1'b0;	//assert a_delay low
end
else if (a == 1'b1) begin	//if a is high on a posedge of clk
	a_delay <= 1'b1;	//assert a_delay high
end
end

always @(a,a_delay) begin //generate a_nedg
if ({a,a_delay} == 2'b01) begin//if a is low and a_delay is high
	a_nedg = 1'b1;		//then assert a_nedge high
end
else begin
	a_nedg = 1'b0;		//if not, assert a_nedge low
end
end

always @(posedge clk) begin	//count process
if (!rst_bar) begin		//if resetted (active low)
	count <= 0;		//then reset count to 0
end
else if (a_nedg == 1'b1) begin	//start counting when a_nedg is detected
	count <= 1;		//count is hard set to 1 rather than incremented 
end				//b/c if another a_nedg is detected shortly after the first one, it needs to be counted from the beginning
else if (count >= 1 && count < DURATION) begin//increment count if greater than or equal to 1 and less than DURATION
	count <= count + 1;
end
else if (count == DURATION) begin//if count has reached DURATION time, then reset count to 0
	count <= 0;
end
end

always @(posedge clk) begin //output (qbar,term) process
if (!rst_bar) begin		//if reset (active low), reset the outputs
	qbar_temp <= 1'b1;	//assert qbar high (qbar is active low, so reset value of qbar is 1)
	term_temp <= 1'b0;	//assert term low
end
else if (count == 0) begin	//if count is 0
	term_temp <= 1'b0;	//term is low
	if (a == 1'b1)		//if a is high
		qbar_temp <= 1'b1;//then qbar is high, this is to ensure if a key press is held
end				//then qbar will be the previous value, which would be qbar being low, if not held, then it will be high
else if ((count >= 1 && count < 500000) || (count > 500000 && count < DURATION)) begin//if count is not 500000 and greater than or equal
	qbar_temp <= 1'b0;					//to 1 but less than DURATION, then qbar and term are low
	term_temp <= 1'b0;
end
else if (count == 500000 && a == 1'b0) begin//if debounce is in the make bounce (a is low)
	term_temp <= 1'b1;	//and count is 5000000, then assert term high
end
end
assign qbar = qbar_temp;
assign term = term_temp;
endmodule 
