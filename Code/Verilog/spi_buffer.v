module spi_buffer(
input clk,
input rst_bar,
input [3:0] key_code,
input term,
input transfer_done,
output [7:0] d,
output busy,
output [1:0] status_ctr);

reg [3:0] d_temp [0:1];
reg read_ptr = 1'b0;
reg write_ptr = 1'b0;
reg [1:0] status_ctr_temp = 2'b00;
reg busy_temp = 1'b0;


always @(posedge clk) begin //reading buffer (out from buffer)
if (!rst_bar) begin
	read_ptr = 1'b0;
end
else begin
	if (transfer_done) begin
		read_ptr <= ~read_ptr;
	end
end
end

always @(posedge clk) begin //writing into buffer (write into buffer)
if (!rst_bar) begin
	write_ptr = 1'b0;
end
else
	if (term && status_ctr < 2) begin
		d_temp[write_ptr] <= key_code;
		write_ptr <= ~write_ptr;
	end
end

always @(posedge clk) begin //status
if (!rst_bar) begin
	busy_temp <= 1'b0;
	status_ctr_temp <= 0;
end
else begin
	if (term) begin
		if (status_ctr_temp < 2) begin
			status_ctr_temp <= status_ctr_temp + 1;
			if (status_ctr_temp == 1) begin
				if (transfer_done) begin
					busy_temp <= 1'b0;
					status_ctr_temp <= 1;
				end
				else begin
					busy_temp <= 1'b1;
				end
			end
			else begin
				busy_temp <= 1'b0;
			end
		end
	end
	else if (transfer_done) begin
		busy_temp <= 1'b0;
		if (status_ctr_temp > 0) begin
			status_ctr_temp <= status_ctr_temp - 1;
		end
	end
end
end

assign status_ctr = status_ctr_temp;
assign d = {4'b0000,d_temp[read_ptr]};
assign busy = busy_temp;
endmodule 
