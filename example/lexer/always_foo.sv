// SPDX-License-Identifier: MIT

module always_foo (
  input clk,
  input rst_n,
  input logic [7:0] data,
  output logic [7:0] q
);
  logic [7:0] next_q;

  assign next_q = data;

  always @(posedge clk) begin
    if (!rst_n) begin
      q <= 0;
    end else begin
      q <= next_q;
    end
  end
endmodule
