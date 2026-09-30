# qldpc_decoder_cpp

C++/HLS **research scaffold** for a qLDPC decoder: CMake + Catch2 + a sparse GF(2) micro-benchmark, plus Vitis HLS / Vivado / PetaLinux scripts targeting the AMD ZCU111 board. The BP kernel and HIL verification are placeholders; hardware results are **UNRUN**.

[![License: AGPL v3](https://img.shields.io/badge/License-AGPL_v3-blue.svg)](https://www.gnu.org/licenses/agpl-3.0)
[![C++ CI](https://github.com/sparkainlp-x/qldpc_decoder_cpp/actions/workflows/ci.yml/badge.svg)](https://github.com/sparkainlp-x/qldpc_decoder_cpp/actions/workflows/ci.yml)
[![Status: research prototype](https://img.shields.io/badge/status-research%20prototype-orange.svg)](#what-it-is-not)
[![Hardware results: UNRUN](https://img.shields.io/badge/hardware%20results-UNRUN-lightgrey.svg)](#evidence-tags)
[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.22985527.svg)](https://doi.org/10.5281/zenodo.22985527)

*English documentation. A French translation can be provided on request.*

## What it is

- A minimal C++20 program that links Joschka Roffe's [`ldpc`](https://github.com/quantumgizmos/ldpc) library through CMake `FetchContent`, with OpenMP enabled.
- A Catch2 unit-test target and a GF(2) sparse matrix-vector micro-benchmark (`ldpc_benchmark`) used as a CI regression gate.
- Scripts for an AMD hardware path on the ZCU111 RFSoC board: `hls/` (Vitis HLS kernel skeleton + testbench), `vivado/` (block design), `petalinux/`, `drivers/axi_dma_driver.cpp`, and `hil/hil_benchmark.cpp`, orchestrated by the `Makefile` and an opt-in self-hosted CI job.

## What it is NOT

- **Not** a complete qLDPC decoder on hardware. The HLS kernel (`hls/qldpc_kernel.cpp`) has no belief-propagation message updates yet.
- **Not** a quantum-hardware result. No qubits, QPU, or cryostat were used; `N_QUBITS = 144` in the HLS kernel is a code-length parameter.
- **Not** a measured decoding latency. The micro-benchmark times a single GF(2) mat-vec on a synthetic 32×64 matrix.
- **Not** hardware-ready or production software. HLS synthesis, timing closure, and HIL runs are **UNRUN**.
- `docs/scientific_review.md` is an **AI-assisted internal review**, not independent peer review.

## Quickstart

Requirements: CMake ≥ 3.20, a C++20 compiler, Git, and OpenMP (for example on Debian/Ubuntu: `sudo apt-get install -y g++ cmake libomp-dev git`). The first configure downloads `quantumgizmos/ldpc` into the build directory.

```bash
git clone https://github.com/sparkainlp-x/qldpc_decoder_cpp.git
cd qldpc_decoder_cpp
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/qldpc_decoder
```

Expected output:

```text
qldpc_decoder_cpp: ldpc library loaded.
OpenMP is enabled on the CMake target.
```

`CMakeLists.txt` uses `-O3 -march=native -ffast-math -flto` on non-MSVC compilers, so the binary is tuned to the build machine.

## Tests

```bash
ctest --test-dir build --output-on-failure   # Catch2 unit tests
./build/ldpc_benchmark                        # GF(2) mat-vec micro-benchmark (median / p95 in ns)
```

CI (`software-ci` job in [`ci.yml`](.github/workflows/ci.yml), GitHub-hosted Ubuntu) builds with Ninja, runs the smoke binary, the Catch2 tests, and the micro-benchmark with a regression gate: it fails if the median reaches `LDPC_MAX_LATENCY_NS` (1000 ns). GitHub-hosted runners are shared VMs, so this is an indicative regression check, not a measurement. The `hardware-bitstream-build` job runs only on manual dispatch on a trusted self-hosted ZCU111 runner and has not produced published results.

## Evidence tags

| Item | Tag | Notes |
|---|---|---|
| Software build, Catch2 tests, smoke test | Runs in CI (GitHub-hosted) | |
| GF(2) sparse mat-vec micro-benchmark, ~70 ns median | **REPORTED; host/conditions unspecified** | A single micro-operation on a synthetic 32×64 matrix. **Not** qLDPC decoding latency, **not** an FPGA or end-to-end figure. CPU, compiler, OS, and raw samples were not recorded |
| HLS kernel (`hls/qldpc_kernel.cpp`) | Scaffold | No belief-propagation message updates yet |
| HLS synthesis, 300/400 MHz timing | **TARGET / UNRUN** | Clock periods are set in TCL; no post-route timing report exists |
| HIL benchmark on ZCU111, FER | **UNRUN** | Needs the board and a self-hosted runner. No run has been published |

Tag definitions: [sparkainlp-x/.github](https://github.com/sparkainlp-x/.github#evidence-tags).

### Known limitations

- **`verify_correction()` always returns `true`** (`hil/hil_benchmark.cpp`). Any HIL run would therefore report FER = 0 *by construction*. It must be replaced with a code-aware check (`H · correction = syndrome (mod 2)`, plus a logical-error check against a known injected error) before any FER figure is published.
- The HLS kernel is an integration skeleton, not a complete BP/OSD decoder.

## Relationship to ADR-001

ADR-001 governs the OES-32 residual family (normative: [oes32-residual@b77b612](https://github.com/sparkainlp-x/oes32-residual/tree/b77b61254f15778c6ae221843dceac7a8571158e); Profile A sidecars: [oes32_engine](https://github.com/sparkainlp-x/oes32_engine), [oes32-hls](https://github.com/sparkainlp-x/oes32-hls)). This decoder scaffold is **independent** of that contract: it does not compute or redefine the OES-32 residual. It shares only the ZCU111 TARGET platform with `oes32-hls`.

## Citation

Archived on Zenodo: concept DOI [10.5281/zenodo.22985527](https://doi.org/10.5281/zenodo.22985527) (all versions; resolves to the latest). The v0.1.1 archive is [10.5281/zenodo.22985528](https://doi.org/10.5281/zenodo.22985528).

Citation metadata is in [CITATION.cff](CITATION.cff); GitHub shows a "Cite this repository" button. Please also cite the upstream [`ldpc`](https://github.com/quantumgizmos/ldpc) library by Joschka Roffe.

## License

This software is available under the GNU Affero General Public License v3.0 only (AGPL-3.0-only); see [LICENSE](LICENSE).

Organizations that want to use it in proprietary products or services without AGPL obligations can contact the author about a commercial license via https://sparkainlpx.xyz.

Versions published before 2026-09-29 were released under the MIT License and remain available under those terms.

Copyright (C) 2026 Jean-François Brisson, Spark AI NLP. The upstream `ldpc` library fetched at build time is distributed under its own license.

---

## Build, benchmark and hardware flow (details)

Minimal C++ example that uses Joschka Roffe's [`ldpc`](https://github.com/quantumgizmos/ldpc) library through CMake `FetchContent`. OpenMP and CPU optimisations are enabled for non-MSVC compilers.

### Prerequisites

CMake 3.20 or later, a C++20 compiler, Git, and an OpenMP implementation.

### Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

### Run

```bash
./build/qldpc_decoder
```

### Latency micro-benchmark

The project also builds `ldpc_benchmark`, which times a sparse GF(2) matrix-vector product after a warm-up phase. It runs 31 samples of 1,000 iterations and prints the median and 95th-percentile latency in nanoseconds. The values of about 70 ns observed so far are **REPORTED; host/conditions unspecified** and concern a single micro-operation, not full qLDPC decoding.

```bash
./build/ldpc_benchmark
```

In GitHub Actions the benchmark fails if the median reaches or exceeds `1000 ns` (`1 µs`). The threshold can be changed with the `LDPC_MAX_LATENCY_NS` environment variable. GitHub-hosted runners are virtualised and shared, so this is an indicative regression check, not a measurement on dedicated hardware.

The first configure step downloads the upstream `quantumgizmos/ldpc` repository into the build directory. `CMakeLists.txt` uses `-O3 -march=native -ffast-math -flto` on non-MSVC compilers and `/O2 /arch:AVX2` on MSVC.

> `-march=native` produces a binary tuned to the build machine. To distribute the binary to other CPUs, replace this flag with a portable target architecture.

### AMD Vitis HLS synthesis (UNRUN)

The `hls/` directory contains the `qldpc_decode_kernel` kernel, its testbench and the `run_hls.tcl` script targeting the RFSoC ZCU111 (`xczu28dr-ffvg1517-2-e`) at 300 MHz (**TARGET**). From an environment with Vitis HLS 2023.2 installed:

```bash
source /tools/Xilinx/Vitis_HLS/2023.2/settings64.sh
vitis_hls -f hls/run_hls.tcl
```

FPGA synthesis is not run by the standard GitHub Actions CI, because the hosted runner provides neither Vitis HLS nor the AMD HLS libraries. CI does verify the portable C++ sources, the Catch2 tests and the software micro-benchmark.

### Hardware pipeline with Make (UNRUN)

The [`Makefile`](./Makefile) orchestrates the hardware steps when an AMD toolchain is installed:

```bash
make help
make hls          # 300 MHz HLS solution
make hls-400mhz   # 400 MHz HLS solution
make vivado       # Block Design and XSA export
make petalinux    # Linux image and BOOT.BIN
make all          # full chain
```

Source the Vitis/Vivado and PetaLinux environments for your local installation first. The repository provides sources and scripts only; it does not contain an initialised PetaLinux project or the AMD tools.

### Hardware CI on a self-hosted runner (opt-in)

The workflow contains a `hardware-bitstream-build` job that runs only on a self-hosted GitHub runner labelled `self-hosted`, `vivado` and `zcu111`. That runner needs Vitis, Vitis HLS, Vivado, PetaLinux, the required AMD licences, and an initialised PetaLinux project in `petalinux/`.

The hardware job waits for `software-ci` to pass, runs `make all`, and publishes the `.xsa`, `BOOT.BIN`, `image.ub` and `download.bit` files as GitHub Actions artifacts. To protect the local machine it is not triggered by pull requests: changes must first be merged into `main`, or a trusted operator must start the workflow manually.

### Automated HIL test (UNRUN)

> **Status: UNRUN.** No HIL test has been run or published. `verify_correction()` always returns `true` (see "Known limitations" above); until it is replaced, the reported FER is 0 by construction.

The benchmark [`hil/hil_benchmark.cpp`](./hil/hil_benchmark.cpp) performs 100,000 AXI-DMA/FPGA transfers, times each round trip in nanoseconds, exports `latencies_report.csv` and checks the maximum latency.

The `hardware-bitstream-build` job compiles this binary with `BUILD_HIL_BENCHMARK=ON` and runs it with `HIL_MAX_LATENCY_US=100`. The job fails as soon as one measurement exceeds 100 µs and keeps the CSV as the `qldpc-hil-latency-report` artifact.

This test must not run on `ubuntu-latest`: it needs `/dev/mem`, the AXI-DMA controller, a loaded bitstream, udmabuf buffers and a ZCU111 board. FER verification in the HIL file is deliberately an extension point; it must be replaced by the real `H * correction == syndrome` check and, if needed, a logical-error check.

See also [`docs/deployment-zcu111.md`](docs/deployment-zcu111.md) and [`vivado/README.md`](vivado/README.md).
