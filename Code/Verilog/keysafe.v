/*Description: Structual module of the Debounced
Double-Buffered Serial Key Matrix Scanner. Uses
modules 'key_encoder', 'debounce', 'scan_rate',
'row_scanner', 'row_buffer' and 'spi_transfer'.

Parameter: 50 MHz clock (20ns)
*/

module keysafe(
input clk,
input rst_bar,
input [3:0] col_in,
input sck,
input ss_bar,
output [3:0] row_out,
output dav,
output busy,
output miso);
/*output rst*/

wire [3:0] qout_row_scanner_out;
wire [3:0] key_code_key_encoder_out;
wire kp_bar_key_encoder_out;
wire qbar_debounce_out;
wire term_debounce_out;
wire q_scan_rate_out;

key_encoder key_encoder(
.col_in(col_in),
.row_in(qout_row_scanner_out),
.key_code(key_code_key_encoder_out),
.kp_bar(kp_bar_key_encoder_out));

debounce #(.DURATION(1000000)) debounce(
.a(kp_bar_key_encoder_out),
.clk(clk),
.rst_bar(rst_bar),
.qbar(qbar_debounce_out),
.term(term_debounce_out));

scan_rate #(.prescaler(8)) scan_rate(
.clk(clk),
.reset_bar(rst_bar),
.q(q_scan_rate_out));

row_scanner row_scanner(
.clk(clk),
.rst_bar(rst_bar),
.en1(qbar_debounce_out),
.en2(q_scan_rate_out),
.qout(qout_row_scanner_out));

row_buffer row_buffer(
.row_in(qout_row_scanner_out),
.row_driver(row_out));

spi_interface spi(
.clk(clk),
.rst_bar(rst_bar),
.ss_bar(ss_bar),
.sck(sck),
.key_code(key_code_key_encoder_out),
.term(term_debounce_out),
.dav(dav),
.busy(busy),
.miso(miso)
/*.rst(rst)*/);

endmodule 
