/*Description: Will receive the key_code byte and
notify the master (Arduino Zero) through the dav 
signal to initiate the SPI transmission. Once SPI 
starts(via the SS_bar going low), SPI_transfer 
transfer's the key_code bit by bit on the miso
line, following the SPI protocol in mode 0. 

Parameters: 50 Mhz clk (20ns)
*/
module spi_transfer(
input clk,
input rst,
input [7:0] key_code,
input ssbar_synced_ne,
input ssbar_synced_pe,
input sck_synced,
input [1:0] status_ctr,
output dav,
output transfer_done,
output miso);

reg dav_temp;
reg miso_temp = 1'bZ;
reg transfer_done_temp = 1'b0;
integer count = 8;
reg [1:0] STATE = 2'b00;

//This process will transfer the new untransmitted
//key_code byte bit by bit in mode 0, i.e data sampled
//on rising edge of SCK and data changes on falling
//edge of SCK. 
always @(posedge clk) begin
if (rst) begin			//if resetted, miso becomes Z,
	miso_temp <= 1'bZ;	//also reset count back to 8
	count <= 8;
end
else begin	//if ss_bar rising edge is detected, this means
if (ssbar_synced_pe) begin//byte transfer is done, so reset
	miso_temp <= 1'bZ;//miso back to Z and count to 8
	count <= 8;	
end		//if ss_bar falling edge OR sck falling edge is detected,
else if (ssbar_synced_ne || sck_synced) begin//and count is not equal to 0,
	if (count > 0) begin	//then transfer the MSB first, and then
		miso_temp <= key_code[count-1];//every other MSB in descending
		count <= count - 1;	    //order. 
	end
	else begin	//if count is 0, then miso is set to Z, but
		miso_temp <= 1'bZ;//count should not be resetted, count			
	end		//should only resetted once a rising edge is detected on ss_bar
end
end
end

always @(posedge clk) begin
if (rst) begin
	transfer_done_temp <= 1'b0;
end
else begin
	if (count == 0 && sck_synced == 1'b1) begin
		transfer_done_temp <= 1'b1;
	end
	else begin
		transfer_done_temp <= 1'b0;
	end
end
end

//This process handles the handshaking between master
//and slaveassertion of dav, i.e. the signal responsible
//for the letting the master know to initiate the
//transmission using a state machine. Dav will only
//be asserted if there are untransmitted key_codes via 
//from input signal 'status_ctr'. 
always @(posedge clk) begin
if (rst) begin	//if resetted, dav is set back to 0
	dav_temp <= 1'b0;//and the STATE is back to 0
	STATE <= 2'b00;
end
else begin//if not resetted, then proceed into the state machine
case (STATE)
	2'b00 : begin//initial/STATE 0, if status_ctr is greater an 1,
		if (status_ctr > 0) begin//then assert dav and go to 
			dav_temp <= 1'b1;//STATE 1. 
			STATE <= 2'b01;
		end
		else begin//if status_ctr is 0, dav is 0
			dav_temp <= 1'b0;
		end
	end
	2'b01 : begin//STATE 1, once SPI transmission is initiated from ss_bar 
		if (ssbar_synced_ne) begin//being asserted, set dav low
			dav_temp <= 1'b0;
		end//dav will be low for the remainder of the transmission
		else if (ssbar_synced_pe) begin//once transmission is finished, (ss_bar
			STATE <= 2'b00; //being high, go back to STATE 0 to check
		end		//if there are untransmitted key_codes. 
	end
endcase
end
end

assign miso = miso_temp;
assign dav = dav_temp;
assign transfer_done = transfer_done_temp;
endmodule 
