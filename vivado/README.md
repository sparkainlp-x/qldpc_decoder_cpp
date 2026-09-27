# Vivado Block Design for the RFSoC (UNRUN)

> **Status: UNRUN.** These scripts have not been run in CI and no timing or implementation report is published. Frequencies below are **TARGET** values.

The script [`create_bd.tcl`](./create_bd.tcl) generates the `system_bd` Block Design for an AMD Zynq UltraScale+ RFSoC ZCU111 (`xczu28dr-ffvg1517-2-e`). It adds the IP repository produced by Vitis HLS and instantiates the RFSoC processing system, the RF data converter, AXI4-Stream FIFOs and the `qldpc_decode_kernel` core.

The script assumes that the Vitis HLS project has already been generated, with the IP available at:

```text
./qldpc_hls_project/solution_300mhz/impl/ip
```

From a correctly configured AMD Vivado environment, run:

```bash
vivado -mode batch -source vivado/create_bd.tcl
```

The script then runs `validate_bd_design` and saves the Block Design. Generation depends on the installed AMD/Xilinx IP versions and cannot be validated in the standard software CI, which provides neither Vivado nor the required FPGA licences.

## 400 MHz variant

[`../hls/run_hls_400mhz.tcl`](../hls/run_hls_400mhz.tcl) creates a `solution_400mhz` solution with a `2.500 ns` target period. [`optimize_timing_400mhz.tcl`](./optimize_timing_400mhz.tcl) enables retiming and performance-oriented implementation strategies and writes `timing_400mhz_report.txt`.

```bash
vitis_hls -f hls/run_hls_400mhz.tcl
vivado -mode batch -source vivado/optimize_timing_400mhz.tcl
```

The 400 MHz target is met only if the post-route report shows WNS ≥ `0 ns`. The optimisation directives do not guarantee the frequency: closure depends on actual place-and-route, tool version, clock constraints and the exact Block Design configuration.
