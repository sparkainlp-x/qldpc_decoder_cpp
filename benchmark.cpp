#include "ldpc.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace {
constexpr int kRows = 32;
constexpr int kColumns = 64;
constexpr int kWarmupIterations = 1000;
constexpr int kIterationsPerSample = 1000;
constexpr int kSamples = 31;

long long threshold_ns()
{
    if (const char* value = std::getenv("LDPC_MAX_LATENCY_NS")) {
        return std::strtoll(value, nullptr, 10);
    }
    return 1000;
}
} // namespace

int main()
{
    cygf2_sparse parity_check(kRows, kColumns);
    for (int row = 0; row < kRows; ++row) {
        std::vector<int> columns{row % kColumns, (row + 7) % kColumns,
                                 (row + 19) % kColumns};
        parity_check.csr_row_insert(row, columns);
    }

    std::vector<std::uint8_t> input(kColumns, 1);
    std::vector<std::uint8_t> output(kRows, 0);
    volatile std::uint8_t checksum = 0;

    for (int i = 0; i < kWarmupIterations; ++i) {
        parity_check.mulvec(input, output);
        checksum ^= output[static_cast<std::size_t>(i) % output.size()];
    }

    std::vector<double> latencies_ns;
    latencies_ns.reserve(kSamples);
    for (int sample = 0; sample < kSamples; ++sample) {
        const auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < kIterationsPerSample; ++i) {
            parity_check.mulvec(input, output);
            checksum ^= output[static_cast<std::size_t>(i) % output.size()];
        }
        const auto elapsed = std::chrono::steady_clock::now() - start;
        const auto elapsed_ns =
            std::chrono::duration<double, std::nano>(elapsed).count();
        latencies_ns.push_back(elapsed_ns / kIterationsPerSample);
    }

    std::sort(latencies_ns.begin(), latencies_ns.end());
    const double median_ns = latencies_ns[latencies_ns.size() / 2];
    const double p95_ns = latencies_ns[static_cast<std::size_t>(kSamples * 0.95)];
    const long long max_latency_ns = threshold_ns();

    std::cout << "LDPC GF(2) sparse matvec benchmark\n"
              << "matrix=" << kRows << "x" << kColumns
              << ", iterations=" << kSamples * kIterationsPerSample << "\n"
              << "median_ns=" << median_ns << "\n"
              << "p95_ns=" << p95_ns << "\n"
              << "threshold_ns=" << max_latency_ns << "\n"
              << "checksum=" << static_cast<int>(checksum) << "\n";

    if (median_ns >= max_latency_ns) {
        std::cerr << "FAIL: median latency is not sub-microsecond.\n";
        return 1;
    }

    std::cout << "PASS: median latency is sub-microsecond.\n";
    return 0;
}
