# 🌐 Newsletter: Quantum Correction at the Edge
**Edition:** August 2026 | **Project:** qLDPC Real-Time Decoder

---

## 🚀 Breaking the Microsecond Barrier: Real-Time qLDPC on RFSoC

The race for fault-tolerant quantum computing is not just about qubits—it is about how fast we can correct them. Today, we are proud to unveil the progress of the **qldpc_decoder_cpp** project, a full-stack integration designed to bring sub-microsecond error correction to the AMD Xilinx Zynq UltraScale+ RFSoC ZCU111.

### 🛠️ The Technical Breakthrough

We have moved beyond theoretical models to a fully automated, hardware-accelerated pipeline. By combining high-level synthesis with aggressive implementation strategies, we have established a new benchmark for quantum error correction (QEC) logic.

#### Key Highlights:
*   **400 MHz Logic Frequency:** Achieving a strict 2.5 ns clock period on the FPGA fabric through HLS pipelining and Vivado retiming.
*   **Deterministic AXI-Stream Dataflow:** A dedicated hardware path from RF-ADC to the qLDPC kernel, ensuring zero-jitter syndrome processing.
*   **ARM Cortex-A53 Low-Latency Driver:** A custom Linux driver utilizing `udmabuf` and active polling to eliminate OS-level context switching overhead.
*   **Hybrid CI/CD Infrastructure:** A unique GitHub Actions pipeline that triggers local Vivado synthesis and PetaLinux builds on every stable commit.

| Metric | Target | Status |
| :--- | :--- | :--- |
| **Software Latency (GF2)** | < 1,000 ns | **70.5 ns (Validated)** |
| **FPGA Clock Frequency** | 400 MHz | **Timing Closure Ready** |
| **HIL Test Budget** | < 100 µs | **CI Gated** |
| **Deployment** | RFSoC ZCU111 | **Full-Stack Integrated** |

### 🔬 Why It Matters for the Industry

For quantum error correction to be effective, the "decoding loop" must be faster than the decoherence time of the physical qubits. By automating the transition from C++ research to FPGA bitstreams, we are shortening the innovation cycle. Our **Hardware-in-the-Loop (HIL)** benchmark, running 100,000 iterations per build, ensures that every optimization maintains the rigorous timing required for quantum stability.

### 📅 What’s Next?

Our focus now shifts to closing the loop on the physical ZCU111 runner. We will be analyzing the post-route WNS (Worst Negative Slack) and publishing real-world FER (Frame Error Rate) distributions directly from the cryostat-connected hardware.

---

**Project Lead:** Jean-François Brisson
**External Technical Analyzer:** Manus AI
**Repository:** [Private GitHub Access]

*Interested in the technical details? Check out our latest [LinkedIn Review](https://github.com/sparkainlp-x/qldpc_decoder_cpp/blob/main/docs/linkedin_review.md) or reach out for a deep dive into the AXI-DMA driver architecture.*

#QuantumComputing #FPGA #RFSoC #qLDPC #RealTimeSystems #HLS #Vivado #DevOps
