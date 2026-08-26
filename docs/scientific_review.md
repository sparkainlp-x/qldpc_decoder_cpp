# Scientific Review of `qldpc_decoder_cpp`

**External technical analysis — Manus AI**  
**Repository:** [sparkainlp-x/qldpc_decoder_cpp](https://github.com/sparkainlp-x/qldpc_decoder_cpp)

## Executive assessment

The repository presents a thoughtful end-to-end engineering scaffold for moving a quantum-LDPC decoding concept from C++ software into an AMD RFSoC-oriented FPGA workflow. Its strongest contribution is architectural: CMake-based software validation, Vitis HLS sources, Vivado automation, PetaLinux integration, an ARM AXI-DMA driver, and a hybrid GitHub Actions pipeline are kept in one versioned project.

The repository should nevertheless be described as **laboratory-ready infrastructure**, not as a scientifically validated FPGA qLDPC decoder. The software microbenchmark is real but narrow; the HLS kernel currently behaves as an integration skeleton; and the HIL benchmark contains a placeholder correction-verification function. The distinction matters for any paper, grant, press release, or magazine article.

## What is demonstrably valuable

The repository has a coherent separation between software CI and hardware CI. The software job runs on a GitHub-hosted runner and performs CMake configuration, compilation, a smoke test, Catch2 tests, and a latency benchmark. The hardware job is gated on the software job and targets a self-hosted runner labelled `self-hosted`, `vivado`, and `zcu111`. This is a sensible pattern for proprietary FPGA tools and physical-board tests.

The project also contains a reproducible intent for timing closure. The 300 MHz solution uses a 3.33 ns target period, while the 400 MHz variant uses a 2.50 ns target period and adds retiming and performance-oriented implementation strategies. These are valid engineering targets, but a target clock period is not equivalent to achieved post-route timing. The evidence required is a Vivado timing report with WNS, TNS, clock uncertainty, and the exact tool/device configuration.

| Area | Evidence visible in repository | Scientific interpretation |
|---|---|---|
| Software build | CMake, Release configuration, OpenMP, Catch2 | Reproducible software integration |
| Software benchmark | Sparse GF(2) matrix-vector microbenchmark | Useful kernel-level measurement, not full decoder latency |
| HLS | `qldpc_kernel.cpp`, 300/400 MHz TCL flows | Hardware implementation scaffold |
| Vivado | Block Design and timing-optimization scripts | Automation intent; post-route evidence still required |
| PetaLinux | Device Tree, deployment notes, ARM driver | Deployment scaffold requiring board-specific verification |
| HIL | 100,000-iteration benchmark and CSV export | Test harness exists; hardware result is not yet demonstrated |
| CI | Successful software runs; hardware job configured | Hybrid validation architecture |

## Benchmark interpretation

The reported software results are approximately 70–76 ns median and 81–87 ns at p95 for a sparse GF(2) operation on a synthetic 32×64 matrix. Those values support the claim that the selected software micro-operation is sub-microsecond on the tested host configuration. They do **not** establish a 70 ns qLDPC decoding latency, an FPGA latency, or an end-to-end host-to-FPGA latency.

A publication-quality benchmark should report the processor model, compiler version, operating-system version, CPU affinity, number of repetitions, warm-up policy, clock source, raw samples, and confidence intervals. It should also compare the optimized implementation with a defined baseline. Without those controls, the numbers are useful engineering observations but not yet a portable performance claim.

## HLS and correctness review

The current HLS source requires a major scientific qualification. The kernel initializes `estimated_error` and `correction_mask` to zero, reads one syndrome, and iterates over a loop that checks a condition without updating belief-propagation messages. The output correction is therefore not evidence of a complete BP decoder. In addition, the helper compares only a subset of the error vector against the syndrome dimensions, which is not a substitute for evaluating the code's parity-check equations.

The HIL benchmark has a similar limitation: `verify_correction` currently returns `true` unconditionally. Consequently, the reported FER is always zero regardless of the returned correction. This must be replaced by a code-aware check, such as verifying the syndrome relation `H · correction = syndrome (mod 2)` and separately measuring logical failure against a known injected error.

## CI and security review

The hybrid CI design is appropriate, especially because a self-hosted runner can expose laboratory infrastructure. The current workflow correctly prevents the hardware job from running on `pull_request` events and keeps it dependent on the software job. GitHub's security guidance recommends avoiding untrusted code on self-hosted runners and pinning third-party actions to full commit SHAs [3]. The workflow should therefore be hardened further by pinning `actions/checkout` and `actions/upload-artifact`, isolating the runner, clearing workspaces, and using a dedicated runner group with minimal network access.

The GitHub API history shows successful software-oriented runs, while some later runs are queued or marked `action_required`. The public status should therefore be reported precisely: **software CI has demonstrated successful runs; the hardware path is configured but requires an available and authorized ZCU111 runner**.

## Recommendations before claiming hardware performance

| Priority | Required action | Acceptance evidence |
|---|---|---|
| 1 | Implement the actual BP/OSD/CS update equations | Unit tests against a reference decoder and syndrome checks |
| 2 | Replace `verify_correction` placeholder | Nonzero-error Monte Carlo with reproducible FER/logical-error results |
| 3 | Confirm AXI-DMA addresses and cache coherency | Device Tree, register map, and board log |
| 4 | Run HLS and Vivado on the target toolchain | HLS reports and post-route WNS/TNS reports |
| 5 | Execute HIL on ZCU111 | Raw CSV, p50/p95/p99/max, sample count, firmware/bitstream hashes |
| 6 | Separate illustrative from measured claims | Versioned report generated by CI |

## Conclusion

This is a promising **integration and validation framework** for experimental qLDPC hardware acceleration. Its engineering novelty lies in connecting software, HLS, FPGA implementation, embedded Linux, DMA transport, and CI governance in one workflow. Its scientific status is one step earlier than the strongest public wording currently suggests: the repository is ready to support laboratory validation, but it does not yet prove a complete qLDPC decoder, sub-20 µs FPGA execution, 400 MHz timing closure, or a nonzero-information FER result.

## References

[1]: https://github.com/sparkainlp-x/qldpc_decoder_cpp "qldpc_decoder_cpp repository"

[2]: https://arxiv.org/abs/2103.06309 "Breuckmann and Eberhardt, Quantum Low-Density Parity-Check Codes, 2021"

[3]: https://docs.github.com/en/actions/reference/security/secure-use "GitHub Docs, Secure use reference"

[4]: https://github.com/quantumgizmos/ldpc "quantumgizmos/ldpc repository"
