`timescale 1ns/1ps

module gpu_top_tb;
    reg clk;
    wire test_out;

    gpu_top uut (
        .clk(clk),
        .test_out(test_out)
    );

    initial begin
        clk = 0;
        forever #5 clk = ~clk; // 100 MHz clock
    end

    initial begin
        // Monitor test_out
        $monitor("Time: %0t | test_out: %b", $time, test_out);

        // Run simulation for a short time
        #100;

        // End simulation
        $finish;
    end
endmodule
