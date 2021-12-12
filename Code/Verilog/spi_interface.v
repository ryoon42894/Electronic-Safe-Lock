/*Description: Structual module of the SPI portion of the
Debounced Double Buffered Key Matrix Scanner. Uses
modules 'spi_buffer', 'spi_synchronizer', 'spi_transfer'.

Parameter: 50 MHz clock (20ns)
*/

module spi_interface(
input clk,
input rst_bar,
input ss_bar,
input sck,
input [3:0] key_code,
input term,
output miso,
output dav,
output busy
/*output rst*/);

wire [7:0] d;
wire [1:0] status_ctr;
wire transfer_done;
wire rst;
wire ssbar_synced_ne;
wire ssbar_synced_pe;
wire sck_synced;

spi_buffer buffer(
.clk(clk),
.rst_bar(rst_bar),
.term(term),
.transfer_done(transfer_done),
.key_code(key_code),
.busy(busy),
.d(d),
.status_ctr(status_ctr));

spi_synchronizer synchronizer(
.clk(clk),
.rst_bar(rst_bar),
.ss_bar(ss_bar),
.sck(sck),
.ssbar_synced_ne(ssbar_synced_ne),
.ssbar_synced_pe(ssbar_synced_pe),
.sck_synced(sck_synced),
.rst(rst));

spi_transfer transfer(
.clk(clk),
.rst(rst),
.key_code(d),

.ssbar_synced_pe(ssbar_synced_pe),
.ssbar_synced_ne(ssbar_synced_ne),
.sck_synced(sck_synced),
.status_ctr(status_ctr),
.dav(dav),
.transfer_done(transfer_done),
.miso(miso));

endmodule
