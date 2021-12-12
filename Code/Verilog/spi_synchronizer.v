/*Description: Crosses rst_bar, ss_bar, and 
sck into the FGPA clock domain(50Mhz, 20ns). 
An active high pulse wide signal is outputted 
that represents ss_bar's positive edge, ss_bar's 
negative edge and sck's negative edge. For rst_bar,
the entire signal will be crossed over to the
FPGA clock domain, except the signal would be
active high. 

Parameter: 50 Mhz clk (20ns)
*/
module spi_synchronizer(
input clk,
input rst_bar,
input ss_bar,
input sck,
output ssbar_synced_ne,
//output ssbar_synced,
output ssbar_synced_pe,
output sck_synced,
output rst);

reg rst_temp;
reg ssbar_nedg = 1'b0;
reg ssbar_pedg = 1'b1;
reg sck_nedg = 1'b0;
reg ssbar_synced_ne_temp;
reg ssbar_synced_temp;
reg ssbar_synced_pe_temp;
reg sck_synced_temp;
integer count = 0;

//The entire rst_bar signal is crossed over
//to the FPGA clock domain. Except, the new 
//signal is active high when rst_bar is
//asserted. 
always @(posedge clk) begin
rst_temp <= ~rst_bar;
end

//Active high pulse signal of ss_bar's negative 
//edge crossed over to the FPGA clock domain.
//This is achieved by comparing the actual ss_bar
//with register, ssbar_nedg; which is the previous 
//value of ss_bar.
always @(posedge clk) begin
ssbar_synced_ne_temp <= 1'b0;	//Set pulse wide register of ss_bar's nedge to 0
if (ssbar_nedg == 1'b1 && ss_bar == 1'b0) begin//if previous ss_bar value is 1, and actual ss_bar is 0
	ssbar_synced_ne_temp <= 1'b1;	//then assert pulse wide register of ss_bar's nedge,
	ssbar_nedg <= ss_bar;		//b/c this means ss_bar went from 1 to 0, i.e. falling edge
end					//also have previous value equal to actual ss_bar
else if (ssbar_nedg != ss_bar) begin//if previous ss_bar value does not equal actual ss_bar
	ssbar_nedg <= ss_bar;	//set previous ss_bar value equal to actual ss_bar 
end
end

//Active high pulse signal of ss_bar's positive 
//edge crossed over to the FPGA clock domain.
//This is achieved by comparing the actual ss_bar
//with register, ssbar_pedg; which is the previous 
//value of ss_bar.
always @(posedge clk) begin	
ssbar_synced_pe_temp <= 1'b0;	//Set pulse wide register of ss_bar's pedge to 0	
if (ssbar_pedg == 1'b0 && ss_bar == 1'b1) begin//if previous ss_bar value is 0, and actual ss_bar is 1
	ssbar_synced_pe_temp <= 1'b1;	//then assert pulse wide register of ss_bar's nedge,
	ssbar_pedg <= ss_bar;		//b/c this means ss_bar went from 0 to 1, i.e. rising edge
end					//also have previous value equal to actual ss_bar
else if (ssbar_pedg != ss_bar) begin//if previous ss_bar value does not equal actual ss_bar
	ssbar_pedg <= ss_bar;		//set previous ss_bar value equal to actual ss_bar 
end
end

//Active high pulse signal of sck's negative 
//edge crossed over to the FPGA clock domain.
//This is achieved by comparing the actual sck
//with register, sck_nedg; which is the previous 
//value of sck.
always @(posedge clk) begin
sck_synced_temp <= 1'b0;	//Set pulse wide register of sck's nedge to 0
if (sck_nedg == 1'b1 && sck == 1'b0) begin//if previous sck value is 1, and actual sck is 0
	sck_synced_temp <= 1'b1;//then assert pulse wide register of sck's nedge,	
	sck_nedg <= sck;	//b/c this means sck went from 1 to 0, i.e. falling edge	
end				//also have previous value equal to actual sck	
else if (sck_nedg != sck) begin	//if previous sck value does not equal actual sck 
	sck_nedg <= sck;	//set previous sck value equal to actual sck 
end
end

assign rst = rst_temp;
assign ssbar_synced_ne = ssbar_synced_ne_temp;
assign ssbar_synced_pe = ssbar_synced_pe_temp;
assign sck_synced = sck_synced_temp;
endmodule 
