// SPDX-License-Identifier: MIT

module module_leaf #(
  parameter WIDTH = 8
) (
  input clk,
  input logic [WIDTH-1:0] data,
  output logic [WIDTH-1:0] q
);
  assign q = data;
endmodule

module module_foo (
  input clk,
  input logic [3:0] data,
  output logic [3:0] q
);
  module_leaf #(.WIDTH(4)) u_leaf (
    .clk(clk),
    .data(data),
    .q(q)
  );
endmodule
