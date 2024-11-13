module gpu(
    input wire clk,
    output wire test_out
);

initial begin
    $display("Hello, World!");
end

shader_core core(
    .test_input(1'b1),
    .test_output(test_out)
);

endmodule
