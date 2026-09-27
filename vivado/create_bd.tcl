# ==============================================================================
# Vivado Block Design generation - Zynq UltraScale+ RFSoC & qLDPC (UNRUN in CI)
# ================================================================================

create_project qldpc_vivado_bd ./qldpc_vivado_bd \
    -part xczu28dr-ffvg1517-2-e -force
create_bd_design "system_bd"

# IP repository produced by Vitis HLS.
set_property ip_repo_paths \
    ./qldpc_hls_project/solution_300mhz/impl/ip [current_project]
update_ip_catalog

# Processeur Zynq UltraScale+ RFSoC.
create_bd_cell -type ip -vlnv xilinx.com:ip:zynq_ultra_ps_e:3.4 zynq_ps
apply_bd_automation \
    -rule xilinx.com:bd_rule:zynq_ultra_ps_e \
    -config {apply_clk_trig "true"} [get_bd_cells zynq_ps]

# Convertisseur RF ADC/DAC.
create_bd_cell -type ip -vlnv xilinx.com:ip:usp_rf_data_converter:2.6 \
    rf_data_converter

# qLDPC kernel synthesised by Vitis HLS.
create_bd_cell -type ip \
    -vlnv sparkainlp:user:qldpc_decode_kernel:1.0 qldpc_decoder

# FIFOs AXI4-Stream.
create_bd_cell -type ip -vlnv xilinx.com:ip:axis_data_fifo:2.0 axis_fifo_in
create_bd_cell -type ip -vlnv xilinx.com:ip:axis_data_fifo:2.0 axis_fifo_out

# 300 MHz clock (TARGET).
create_bd_cell -type ip -vlnv xilinx.com:ip:clk_wiz:6.0 clk_wiz_300MHz
set_property -dict [list CONFIG.CLKOUT1_REQUESTED_OUT_FREQ {300.000}] \
    [get_bd_cells clk_wiz_300MHz]
connect_bd_net [get_bd_pins zynq_ps/pl_clk0] \
    [get_bd_pins clk_wiz_300MHz/clk_in1]
set system_clk [get_bd_pins clk_wiz_300MHz/clk_out1]

# Syndrome stream: RF ADC -> FIFO -> HLS decoder.
connect_bd_intf_net [get_bd_intf_pins rf_data_converter/m00_axis] \
    [get_bd_intf_pins axis_fifo_in/S_AXIS]
connect_bd_intf_net [get_bd_intf_pins axis_fifo_in/M_AXIS] \
    [get_bd_intf_pins qldpc_decoder/in_syndrome]

# Correction stream: HLS decoder -> FIFO -> RF DAC.
connect_bd_intf_net [get_bd_intf_pins qldpc_decoder/out_correction] \
    [get_bd_intf_pins axis_fifo_out/S_AXIS]
connect_bd_intf_net [get_bd_intf_pins axis_fifo_out/M_AXIS] \
    [get_bd_intf_pins rf_data_converter/s00_axis]

# AXI-Lite control from the ARM processor.
apply_bd_automation -rule xilinx.com:bd_rule:axi4 \
    -config { Master "/zynq_ps/M_AXI_HPM0_FPD" Clk "Auto" } \
    [get_bd_intf_pins qldpc_decoder/s_axi_control]

validate_bd_design
save_bd_design
