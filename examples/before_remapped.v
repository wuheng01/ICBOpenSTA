module top (in1,
    in2,
    clk1,
    out);
 input in1;
 input in2;
 input clk1;
 output out;

 wire r1q;
 wire r2q;
 wire u1z;
 wire u1z_inv;
 wire u2z;

 DFF_X1 r1 (.D(in1),
    .CK(clk1),
    .Q(r1q));
 DFF_X1 r2 (.D(in2),
    .CK(clk1),
    .Q(r2q));
 DFF_X1 r3 (.D(u2z),
    .CK(clk1),
    .Q(out));
 OR2_X1 u1 (.A1(r2q),
    .A2(in1),
    .ZN(u1z));
 AND2_X1 u2 (.A1(r1q),
    .A2(u1z_inv),
    .ZN(u2z));
 INV_X1 u3 (.A(u1z),
    .ZN(u1z_inv));
endmodule
