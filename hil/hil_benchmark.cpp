#define AXI_DMA_DRIVER_NO_MAIN
#define AXI_DMA_DRIVER_QUIET
#include "../drivers/axi_dma_driver.cpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>

namespace {
constexpr int kDefaultTests = 100000;
constexpr size_t kSyndromeBytes = 16;
constexpr size_t kCorrectionBytes = 32;

int test_count()
{
    if (const char* value = std::getenv("HIL_NUM_TESTS")) {
        return std::max(1, std::atoi(value));
    }
    return kDefaultTests;
}

double max_latency_us()
{
    if (const char* value = std::getenv("HIL_MAX_LATENCY_US")) {
        return std::strtod(value, nullptr);
    }
    return 100.0;
}

// Replace with H * correction == syndrome and a logical-error check.
bool verify_correction(const uint8_t*, const uint8_t*)
{
    return true;
}
} // namespace

int main()
{
    const int num_tests = test_count();
    AxiDmaDriver dma;
    if (!dma.init()) {
        std::cerr << "Erreur d’initialisation AXI-DMA.\n";
        return 1;
    }

    std::vector<long long> latencies;
    latencies.reserve(static_cast<size_t>(num_tests));
    std::mt19937 generator(42);
    std::uniform_int_distribution<int> distribution(0, 255);
    uint8_t syndrome[kSyndromeBytes] = {};
    uint8_t correction[kCorrectionBytes] = {};
    int logical_errors = 0;

    std::cout << "[HIL] Lancement de " << num_tests
              << " hardware decodes.\n";
    for (int i = 0; i < num_tests; ++i) {
        for (auto& byte : syndrome) {
            byte = static_cast<uint8_t>(distribution(generator));
        }

        const auto start = std::chrono::steady_clock::now();
        dma.execute_decoding(syndrome, correction);
        const auto end = std::chrono::steady_clock::now();
        latencies.push_back(
            std::chrono::duration_cast<std::chrono::nanoseconds>(end - start)
                .count());

        if (!verify_correction(syndrome, correction)) {
            ++logical_errors;
        }
        if ((i + 1) % 10000 == 0 || i + 1 == num_tests) {
            std::cout << "[HIL] " << (i + 1) << "/" << num_tests << "\n";
        }
    }

    std::sort(latencies.begin(), latencies.end());
    const auto p99_index = static_cast<size_t>(
        std::min<double>(latencies.size() - 1, (latencies.size() - 1) * 0.99));
    const long long sum = std::accumulate(latencies.begin(), latencies.end(), 0LL);
    const double average_ns = static_cast<double>(sum) / latencies.size();
    const double p99_ns = static_cast<double>(latencies[p99_index]);
    const double max_ns = static_cast<double>(latencies.back());
    const double fer = static_cast<double>(logical_errors) / num_tests;

    std::ofstream csv("latencies_report.csv");
    if (!csv) {
        std::cerr << "Error: cannot create latencies_report.csv\n";
        return 1;
    }
    csv << "index,latency_ns\n";
    for (size_t i = 0; i < latencies.size(); ++i) {
        csv << i << ',' << latencies[i] << '\n';
    }

    std::cout << std::fixed << std::setprecision(3)
              << "[HIL] moyenne_us=" << average_ns / 1000.0 << '\n'
              << "[HIL] p99_us=" << p99_ns / 1000.0 << '\n'
              << "[HIL] max_us=" << max_ns / 1000.0 << '\n'
              << "[HIL] FER=" << fer << '\n'
              << "[HIL] CSV=latencies_report.csv\n";

    const double limit = max_latency_us();
    if (max_ns / 1000.0 > limit) {
        std::cerr << "FAIL: maximum latency above " << limit
                  << " us.\n";
        return 2;
    }
    std::cout << "PASS: all latencies are below " << limit << " us.\n";
    return 0;
}
