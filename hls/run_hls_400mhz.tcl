# ================================================================================
# qLDPC Vitis HLS synthesis - 400 MHz TARGET (UNRUN in CI)
# ================================================================================

set script_dir [file dirname [file normalize [info script]]]

open_project qldpc_hls_project_400mhz
set_top qldpc_decode_kernel
add_files "$script_dir/qldpc_kernel.cpp" -cflags "-std=c++14 -I$script_dir"
add_files -tb "$script_dir/testbench.cpp" -cflags "-std=c++14 -I$script_dir"

open_solution "solution_400mhz" -flow_target vitis
set_part {xczu28dr-ffvg1517-2-e}
create_clock -period 2.500 -name default

config_interface -m_axi_addr64
config_compile -pipeline_loops 1
config_schedule -effort high

csim_design
csynth_design
cosim_design -trace_level all
export_design -format ip_catalog \
    -description "qLDPC BP-OSD-CS Hardware Decoder Kernel 400 MHz" \
    -vendor "sparkainlp" -version "1.0"

exit
