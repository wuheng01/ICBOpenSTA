module top (in1,
    in2,
    clk1,
    out);
 input in1;
 input in2;
 input clk1;
 output out;

 wire AOI221_X1_6;
 wire INV_X1_3;
 wire INV_X1_4;
 wire OAI21_X1_5;
 wire r1q;
 wire r2q;
 wire u1z;
 wire u1z_inv;
 wire u2z;

 AOI221_X1 AOI221_X1 (.A(OAI21_X1_5),
    .B1(r2q),
    .B2(INV_X1_4),
    .C1(INV_X1_3),
    .C2(in1),
    .ZN(AOI221_X1_6));
 INV_X1 INV_X1 (.A(in1),
    .ZN(INV_X1_4));
 OAI21_X1 OAI21_X1 (.A(r1q),
    .B1(INV_X1_3),
    .B2(INV_X1_4),
    .ZN(OAI21_X1_5));
 DFF_X1 r1 (.D(in1),
    .CK(clk1),
    .Q(r1q));
 DFF_X1 r2 (.D(in2),
    .CK(clk1),
    .Q(r2q));
 DFF_X1 r3 (.D(u2z),
    .CK(clk1),
    .Q(out));
endmodule
