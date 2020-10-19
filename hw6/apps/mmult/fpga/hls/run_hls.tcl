# Create a project
open_project -reset proj_mmult

# Add design files
add_files MMult.cpp
add_files MMult.h

# Add test bench & files
add_files -tb testbench.cpp

# Set the top-level function
set_top mmult_fpga

# ########################################################
# Create a solution
open_solution -reset solution1 -flow_target vitis
# Define technology and clock rate
set_part  {xczu3eg-sbva484-1-e}
create_clock -period 5

config_compile -dump_cfg=0 -name_max_length 80 -no_signed_zeros=0 -pipeline_loops 0 -pipeline_style stp -pragma_strict_mode=0 -unsafe_math_optimizations=0
csim_design
csynth_design
config_export -format xo -output ../../kernel.xo -rtl verilog
export_design -rtl verilog -format xo -output ../../kernel.xo

exit
