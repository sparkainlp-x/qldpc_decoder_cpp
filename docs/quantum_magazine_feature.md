# The Quantum Link to Reality

## How a qLDPC decoder is moving from parity-check mathematics to FPGA laboratory hardware

**By Manus AI — External Technical Analysis**

Quantum computing does not only need better qubits. It needs a control loop fast enough to detect, decode, and respond to errors before fragile quantum information is lost. That is the engineering challenge behind `qldpc_decoder_cpp`, an open technical prototype assembled around sparse quantum-LDPC decoding, FPGA acceleration, embedded Linux, and continuous integration.

Quantum low-density parity-check codes are studied as a promising family of quantum error-correcting codes and as an alternative route to fault tolerance alongside the surface code [1]. Their promise, however, creates a practical obligation: decoding cannot remain an abstract equation on a whiteboard. The syndrome must travel through real interfaces, real memory, real clocks, and real software.

> **The equation is compact. The system is not.**
>
> `s = H e (mod 2)` describes the syndrome relation, but a laboratory decoder must turn that relation into deterministic data movement and a verified correction.

## From sparse graphs to silicon

The project packages the path from algorithm to hardware in one private GitHub repository [2]. The C++ layer uses CMake, OpenMP, compiler optimization, unit tests, and a sparse GF(2) benchmark. The hardware layer adds a Vitis HLS kernel, AXI-Stream interfaces, Vivado Block Design scripts, PetaLinux configuration, and an ARM-side AXI-DMA driver.

The architecture is deliberately hybrid. The software pipeline runs on a GitHub-hosted runner, while synthesis and board-level tests are reserved for a self-hosted ZCU111 runner equipped with AMD development tools. In principle, one commit can therefore connect a software change to a bitstream build and a hardware-in-the-loop measurement.

| Signal of ambition | Current project evidence |
|---|---|
| High-throughput hardware | HLS pipeline target with II=1 |
| High clock target | 300 MHz nominal flow and 400 MHz variant |
| Low software overhead | GF(2) microbenchmark in the ~70–76 ns median range |
| Board-level gate | HIL threshold configured at 100 µs |
| Reproducibility | CMake, CTest, GitHub Actions and versioned scripts |

## The benchmark headline — and the important footnote

The software benchmark reports a median around 70–76 ns and p95 around 81–87 ns for a sparse GF(2) matrix-vector operation on a synthetic 32×64 problem. That is a compelling kernel-level result. It is not, by itself, a 70 ns qLDPC decoder, an FPGA measurement, or a complete host-to-FPGA latency.

That distinction is not a weakness. It is what makes the project credible. A serious hardware story must identify exactly what was measured, on which device, with which compiler, and under which timing and sampling conditions. The next step is to publish the raw HIL CSV together with the board identity, bitstream hash, firmware revision, p50/p95/p99/max latency, and logical-error statistics.

## A CI pipeline with a laboratory at the end

The most striking feature is not a single clock number. It is the attempt to make hardware experimentation behave more like modern software engineering. The software job builds, runs Catch2, executes a smoke test, and applies the software latency gate. A protected hardware job then targets the local FPGA laboratory, produces XSA and PetaLinux artifacts, and runs the HIL benchmark when an authorized runner is available.

GitHub explicitly warns that self-hosted runners require careful isolation because workflow code can access the runner environment [3]. The project’s restriction against running the hardware job for external pull requests is therefore an important design decision, not a minor implementation detail.

## What is ready now?

The repository is **ready for laboratory validation**. It is not yet evidence that a complete BP/OSD decoder has closed timing at 400 MHz or achieved sub-20 µs FPGA execution. The current HLS kernel and HIL correctness check still require completion: the decoder update equations must be implemented, and the placeholder correction verifier must be replaced with a code-aware syndrome and logical-error test.

That is the real story—and it is stronger than a slogan. The project has built the bridge from quantum error-correction mathematics to a measurable hardware experiment. The laboratory result is the missing span of that bridge.

## Why this matters

Fault-tolerant quantum computing will depend on systems that are simultaneously mathematical, electrical, embedded, and operational. A decoder that is fast in isolation but difficult to build, deploy, reproduce, or audit will not be enough. The qLDPC project points toward a more complete model: treat the decoder as a living system, and treat its performance claims as artifacts that must survive code review, synthesis, routing, and hardware measurement.

The next headline should come from the ZCU111 itself: a signed bitstream, a post-route timing report, a raw latency distribution, and a verified logical-error curve. Until then, the most accurate description is also the most exciting one:

> **The quantum link to reality is under construction—and its next test is in the laboratory.**

## References

[1]: https://arxiv.org/abs/2103.06309 "Breuckmann and Eberhardt, Quantum Low-Density Parity-Check Codes"

[2]: https://github.com/sparkainlp-x/qldpc_decoder_cpp "qldpc_decoder_cpp GitHub repository"

[3]: https://docs.github.com/en/actions/reference/security/secure-use "GitHub Docs, Secure use reference"
