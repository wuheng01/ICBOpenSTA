#Example of running physical synthesis on small example
#(example2.v)
read_liberty nangate45_fast.lib      
read_verilog example2.v
link_design top
#Set up some constraints
create_clock -name clk1 clk1 -period 1.00
report_clock_properties clk1
#report them
report_checks
#Entry to physical world
write_verilog before_remapped.v
phys_remap nangate45_fast.lib
#write out the resynthesised verilog
write_verilog after_remapped.v



