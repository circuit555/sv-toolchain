// SPDX-License-Identifier: MIT

module generate_foo #(
  parameter WIDTH = 4
) (
  input logic [WIDTH-1:0] data,
  output logic [WIDTH-1:0] q,
  output logic done
);
  logic [WIDTH-1:0] masked_data;

  generate
    genvar i;
    for (i = 0; i < WIDTH; i = i + 1) begin : gen_mask
      assign masked_data[i] = data[i];
      assign q[i] = masked_data[i];
    end
  endgenerate

  assign done = 1;
endmodule
