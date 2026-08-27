# Low-Latency LDPC/qLDPC Decoder Platform

[![C++ CI](https://github.com/sparkainlp-x/qldpc_decoder_cpp/actions/workflows/ci.yml/badge.svg)](https://github.com/sparkainlp-x/qldpc_decoder_cpp/actions/workflows/ci.yml)

A C++20 and AMD FPGA/HLS research platform for developing,
benchmarking, and validating classical LDPC and quantum LDPC
decoding workflows.

---

## Overview

This project provides a research and engineering framework for
low-latency decoding of LDPC and qLDPC-related error-correction
workloads. It combines a portable C++ implementation, sparse
binary matrix operations over GF(2), AMD Vitis HLS acceleration,
Vivado/PetaLinux integration, and hardware-in-the-loop testing.

The project is intended for researchers, FPGA developers, quantum
computing groups, communications engineers, and organizations
investigating hardware-accelerated error correction.

---

## Scope

The current repository is a research and prototype platform. It is
designed to support algorithm development, software benchmarking,
FPGA synthesis experiments, and hardware-in-the-loop validation.

It is not yet presented as a production-ready quantum error-correction
decoder, a complete fault-tolerant quantum-computing system, or a
drop-in replacement for commercial communications IP.

---

## Research Objectives

The project investigates:

- Low-latency sparse matrix operations over GF(2).
- Software implementations of LDPC/qLDPC decoding workflows.
- FPGA-oriented decoder architectures.
- Deterministic latency for real-time error-correction pipelines.
- Hardware/software co-design using AMD RFSoC platforms.
- Reproducible benchmarking and hardware-in-the-loop validation.
- Possible applications in quantum error correction and classical communications.

---

## Potential Applications

Potential application areas include:

- Quantum error-correction research.
- Quantum processor control and decoding experiments.
- FPGA-based communications systems.
- 5G/6G and satellite communications research.
- Optical and high-speed data links.
- Storage and memory error correction.
- Defense and secure communications research.
- University and national-laboratory prototyping.

---

## System Architecture

The project is organized into several implementation layers:

1. **Software layer**
   Portable C++ code for decoder experiments, matrix operations,
   correctness tests, and CPU benchmarking.

2. **HLS layer**
   Synthesizable C++ kernels intended for AMD Vitis HLS and RFSoC
   FPGA targets.

3. **FPGA integration layer**
   Vivado block-design and hardware-export workflows.

4. **Embedded Linux layer**
   PetaLinux configuration and boot-image generation.

5. **Hardware-in-the-loop layer**
   AXI-DMA transfer tests and latency measurement on the target board.

---

## Repository Structure

| Directory or file | Purpose |
|---|---|
| `main.cpp` | Software decoder example or demonstration entry point |
| `benchmark.cpp` | CPU latency benchmark |
| `monte_carlo.cpp` | Monte-Carlo Frame Error Rate simulator |
| `tests/` | Software correctness and regression tests |
| `hls/` | Vitis HLS kernels, testbenches, and synthesis scripts |
| `vivado/` | FPGA block-design and hardware-integration files |
| `drivers/` | Hardware-access or device-driver components |
| `hil/` | Hardware-in-the-loop benchmarks and latency reports |
| `petalinux/` | Embedded Linux configuration |
| `docs/` | Technical documentation and research notes |
| `CMakeLists.txt` | Software build configuration |
| `Makefile` | Software and hardware workflow commands |
| `PERFORMANCE.md` | Benchmark results and performance analysis |

---

## Current Capabilities

- C++20 software build using CMake.
- Integration with the upstream `ldpc` library.
- Sparse binary matrix-vector operations over GF(2).
- CPU benchmarking with median and percentile latency reporting.
- Monte-Carlo Frame Error Rate simulation with configurable BER sweep.
- Catch2-based software testing.
- AMD Vitis HLS source and synthesis workflow.
- RFSoC ZCU111 target configuration.
- Vivado and PetaLinux integration scripts.
- AXI-DMA hardware-in-the-loop benchmark structure.

## Planned Capabilities

- Expanded decoder algorithm support.
- Larger and configurable code families.
- Formal logical-error-rate evaluation.
- Independent reference-model validation.
- Broader FPGA portability.
- Power and resource-utilization reporting.
- Reproducible benchmark datasets and published results.
- Research partnerships and pilot deployments.

---

## Validation Status

| Area | Status |
|---|---|
| Portable C++ compilation | Implemented |
| Software benchmark | Implemented |
| Monte-Carlo FER simulation | Implemented |
| Continuous integration | Implemented |
| Vitis HLS source flow | Prepared |
| Vivado integration | Prepared |
| PetaLinux workflow | Prepared |
| ZCU111 hardware execution | Requires compatible local hardware |
| HIL DMA measurements | Requires configured ZCU111 system |
| Full logical-error-rate study | Future work |
| Production deployment qualification | Not yet completed |

---

## Quick Start

### Prerequisites

- CMake 3.20 or later
- C++20-compatible compiler
- Git
- OpenMP implementation

### Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

### Run the decoder example

```bash
./build/qldpc_decoder
```

### Run tests

```bash
ctest --test-dir build --output-on-failure
```

---

## Latency Benchmark

The `ldpc_benchmark` executable measures a sparse GF(2) matrix-vector
multiplication after a warm-up phase. It runs 31 samples of 1,000
iterations and reports median and 95th-percentile latency in nanoseconds.

```bash
./build/ldpc_benchmark
```

In GitHub Actions, the benchmark fails if the median reaches or exceeds
`1000 ns` (1 µs). The threshold can be changed with the `LDPC_MAX_LATENCY_NS`
environment variable.

---

## Monte-Carlo FER Simulation

The `monte_carlo_sim` executable runs a configurable BER sweep and produces
Frame Error Rate statistics with standard error and 95% confidence intervals.

```bash
./build/monte_carlo_sim \
  --trials 10000 \
  --ber-list 0.001,0.005,0.01,0.02,0.05 \
  --csv fer_curve.csv \
  --json fer_curve.json
```

**Options:**

| Option | Default | Description |
|---|---|---|
| `--trials <N>` | 10000 | Frames per BER point |
| `--ber-list <a,b,c>` | 0.001,0.005,0.01,0.02,0.05 | Comma-separated BER values |
| `--seed <N>` | 42 | MT19937 RNG seed for reproducibility |
| `--csv <path>` | fer_curve.csv | CSV output path |
| `--json <path>` | — | Optional JSON output path |
| `--progress-interval <N>` | 1000 | Progress print interval per BER point |

The CSV output format is: `BER,FER,FramesAtBER,ErrorsAtBER,Timestamp`

See [PERFORMANCE.md](PERFORMANCE.md) for benchmark results and analysis.

---

## Benchmark Interpretation

The software benchmark measures selected sparse GF(2) matrix-vector
operations after a warm-up phase. It is an internal kernel benchmark
and should not be interpreted as complete end-to-end decoder latency.

The hardware-in-the-loop benchmark measures transfer and hardware
execution behavior on a configured FPGA platform. Results depend on
the code parameters, synthesized design, clock frequency, DMA path,
operating system, and board configuration.

All performance claims should identify:

- Code size and matrix dimensions.
- Decoder algorithm and iteration count.
- Target processor or FPGA.
- Clock frequency.
- Number of samples.
- Median, percentile, and maximum latency.
- Whether data-transfer time is included.

---

## AMD Vitis HLS Synthesis

The `hls/` directory contains the `qldpc_decode_kernel`, its testbench,
and `run_hls.tcl` targeting the RFSoC ZCU111 (`xczu28dr-ffvg1517-2-e`)
at 300 MHz. From an environment with Vitis HLS 2023.2 installed:

```bash
source /tools/Xilinx/Vitis_HLS/2023.2/settings64.sh
vitis_hls -f hls/run_hls.tcl
```

FPGA synthesis is not executed by the standard GitHub Actions CI because
the runner does not provide Vitis HLS or AMD HLS libraries. CI validates
portable C++ sources, Catch2 tests, and the software benchmark.

## Hardware Pipeline with Make

The [`Makefile`](./Makefile) orchestrates hardware steps when an AMD
environment is installed:

```bash
make help
make hls          # HLS solution at 300 MHz
make hls-400mhz   # HLS solution at 400 MHz
make vivado       # Block Design and XSA export
make petalinux    # Linux image and BOOT.BIN
make all          # Full chain
```

Load the Vitis/Vivado and PetaLinux environments before use. The
repository provides only sources and scripts; it does not include an
initialized PetaLinux project or AMD tools.

---

## Limitations

- FPGA synthesis requires AMD Vitis, Vivado, and compatible licenses.
- The standard GitHub-hosted CI environment cannot execute the complete
  FPGA workflow.
- Hardware-in-the-loop tests require a configured AMD ZCU111 platform.
- CPU benchmark results are machine-dependent.
- HLS timing estimates are not equivalent to measured board performance.
- The project does not yet provide a complete production-qualified
  quantum error-correction stack.
- Decoder performance depends strongly on code family, matrix size,
  iteration count, and noise assumptions.

---

## Reproducibility

When reporting results, record:

- Git commit identifier.
- Compiler and compiler version.
- CMake version.
- Operating system.
- CPU model or FPGA part number.
- Build type and compiler flags.
- Code dimensions and parameters.
- Decoder configuration.
- Number of trials.
- Random seed, when applicable.
- Whether transfer and initialization time are included.

---

## Technology and Market Position

The project targets the intersection of quantum error-correction research,
FPGA acceleration, and established classical LDPC applications.

Near-term opportunities include research software, FPGA prototyping,
decoder customization, communications demonstrations, and
hardware/software co-development.

Longer-term opportunities may include licensed decoder IP, embedded
quantum-control components, communications accelerators, and
specialized low-latency error-correction systems.

The project is currently positioned as a research and prototype platform.
Commercial deployment would require additional validation, customer
requirements, hardware qualification, licensing review, and support
infrastructure.

---

## Intended Users

This project may be useful to:

- Quantum-computing researchers.
- FPGA and HLS engineers.
- Communications-system developers.
- University laboratories.
- Government and national research organizations.
- Companies developing quantum hardware.
- Companies evaluating low-latency error-correction accelerators.
- Students learning hardware/software decoder co-design.

---

## Roadmap

### Completed or available

- Portable C++ build.
- Software benchmark.
- Monte-Carlo FER simulation.
- CI-oriented test structure.
- HLS kernel structure.
- FPGA workflow scripts.
- Hardware-in-the-loop benchmark structure.

### Next priorities

- Document supported code parameters.
- Add formal decoder correctness tests.
- Publish benchmark methodology.
- Add complete FER and logical-error-rate evaluation.
- Compare software and FPGA results.
- Report FPGA resource utilization.
- Document supported hardware and tool versions.

### Longer-term goals

- Support multiple decoder algorithms.
- Add configurable code families.
- Improve FPGA portability.
- Develop research and industry pilot demonstrations.
- Evaluate licensing and commercial support options.

---

## Dependencies and Attribution

This project uses the `ldpc` software library by Joschka Roffe
through CMake FetchContent. Users should consult the
[upstream repository](https://github.com/quantumgizmos/ldpc) and
license terms for dependency details.

Scientific claims about decoder performance should be accompanied by
the relevant code parameters, benchmark methodology, and references
to published research.

---

## Project Description

This project develops a hardware-oriented software platform for
low-latency LDPC and qLDPC decoding. It combines portable C++
implementations with AMD FPGA/HLS acceleration, embedded Linux
integration, and hardware-in-the-loop validation. The platform is
intended to support research and prototyping in quantum error correction,
high-speed communications, storage, and other systems requiring rapid
error detection and correction.

> **Commercial summary:** We are developing a customizable LDPC/qLDPC
> decoder platform that connects algorithm research with FPGA deployment.
> The system is designed for organizations evaluating deterministic,
> low-latency error-correction architectures for quantum hardware,
> communications, storage, and embedded systems. Initial collaboration
> opportunities include benchmarking, FPGA prototyping, decoder
> customization, and hardware/software integration.

---

## CI and Hardware Testing

The workflow contains a `hardware-bitstream-build` job that runs only on
a self-hosted GitHub runner with labels `self-hosted`, `vivado`, and
`zcu111`. This runner must have Vitis, Vitis HLS, Vivado, PetaLinux,
the necessary AMD licenses, and an initialized PetaLinux project in
`petalinux/`.

The hardware job waits for `software-ci` to succeed, runs `make all`,
then publishes `.xsa`, `BOOT.BIN`, `image.ub`, and `download.bit` as
GitHub Actions artifacts. To protect local hardware, it is not triggered
by pull requests.

The HIL benchmark [`hil/hil_benchmark.cpp`](./hil/hil_benchmark.cpp)
executes 100,000 AXI-DMA/FPGA transfers, measures each round-trip in
nanoseconds, exports `latencies_report.csv`, and validates maximum
latency. The `hardware-bitstream-build` CI job compiles this binary with
`BUILD_HIL_BENCHMARK=ON` and runs it with `HIL_MAX_LATENCY_US=100`.
