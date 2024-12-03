`default_nettype none
`timescale 1ns/1ns

module compute_core(
    input test_input,
    output test_output
    );

assign test_output = ~test_input;

endmodule
