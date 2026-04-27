// SPDX-License-Identifier: MIT

module foo #(
  parameter int N = 8,
  parameter logic [7:0] MASK = N - 1, ALT_MASK = 2,
  parameter type T = logic [3:0], U = bit
) (
  input clk,
  input rst_n,
  output logic [N-1 : 0] bus,
  valid
);
  wire [N-1 : 0] internal_bus;
  logic ready;
endmodule
