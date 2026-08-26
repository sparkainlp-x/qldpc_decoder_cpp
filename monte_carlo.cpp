#include "bp.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <random>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace {

struct SimulationConfig {
    std::size_t trials_per_ber = 10000;
    std::vector<double> ber_points{0.001, 0.005, 0.01, 0.02, 0.05};
    std::uint32_t seed = 42;
    std::string csv_path = "fer_curve.csv";
    std::optional<std::string> json_path;
    std::size_t progress_interval = 1000;
};

struct FerResult {
    double ber = 0.0;
    std::size_t frames = 0;
    std::size_t errors = 0;
    double fer = 0.0;
    double standard_error = 0.0;
    double ci95_low = 0.0;
    double ci95_high = 0.0;
};

constexpr int kRows = 2;
constexpr int kColumns = 4;
constexpr int kMaxIterations = 40;
constexpr double kBpScaling = 0.625;

void print_usage(const char* program_name)
{
    std::cout << "Usage: " << program_name << " [options]\n"
              << "Options:\n"
              << "  --trials <N>              Number of trials per BER (default: 10000)\n"
              << "  --ber-list <a,b,c>        Comma-separated BER values\n"
              << "  --seed <N>                RNG seed (default: 42)\n"
              << "  --csv <path>              CSV output path (default: fer_curve.csv)\n"
              << "  --json <path>             Optional JSON output path\n"
              << "  --progress-interval <N>   Progress print interval per BER (default: 1000)\n"
              << "  --help                    Show this help message\n";
}

bool parse_size(const std::string& raw_value, std::size_t& value)
{
    try {
        const std::size_t parsed = std::stoull(raw_value);
        value = parsed;
        return true;
    } catch (...) {
        return false;
    }
}

bool parse_u32(const std::string& raw_value, std::uint32_t& value)
{
    try {
        const unsigned long parsed = std::stoul(raw_value);
        if (parsed > std::numeric_limits<std::uint32_t>::max()) {
            return false;
        }
        value = static_cast<std::uint32_t>(parsed);
        return true;
    } catch (...) {
        return false;
    }
}

bool parse_ber_list(const std::string& raw_value, std::vector<double>& values)
{
    std::stringstream ss(raw_value);
    std::string token;
    std::vector<double> parsed;
    while (std::getline(ss, token, ',')) {
        if (token.empty()) {
            continue;
        }
        try {
            const double ber = std::stod(token);
            if (ber < 0.0 || ber > 1.0) {
                return false;
            }
            parsed.push_back(ber);
        } catch (...) {
            return false;
        }
    }
    if (parsed.empty()) {
        return false;
    }
    values = std::move(parsed);
    return true;
}

bool parse_arguments(int argc, char** argv, SimulationConfig& config)
{
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg(argv[i]);
        auto require_value = [&](const char* option_name) -> const char* {
            if (i + 1 >= argc) {
                std::cerr << "Missing value for option " << option_name << ".\n";
                return nullptr;
            }
            ++i;
            return argv[i];
        };

        if (arg == "--help") {
            print_usage(argv[0]);
            return false;
        }
        if (arg == "--trials") {
            const char* value = require_value("--trials");
            if (!value || !parse_size(value, config.trials_per_ber) ||
                config.trials_per_ber == 0) {
                std::cerr << "Invalid --trials value.\n";
                return false;
            }
            continue;
        }
        if (arg == "--ber-list") {
            const char* value = require_value("--ber-list");
            if (!value || !parse_ber_list(value, config.ber_points)) {
                std::cerr << "Invalid --ber-list value.\n";
                return false;
            }
            continue;
        }
        if (arg == "--seed") {
            const char* value = require_value("--seed");
            if (!value || !parse_u32(value, config.seed)) {
                std::cerr << "Invalid --seed value.\n";
                return false;
            }
            continue;
        }
        if (arg == "--csv") {
            const char* value = require_value("--csv");
            if (!value) {
                return false;
            }
            config.csv_path = value;
            continue;
        }
        if (arg == "--json") {
            const char* value = require_value("--json");
            if (!value) {
                return false;
            }
            config.json_path = value;
            continue;
        }
        if (arg == "--progress-interval") {
            const char* value = require_value("--progress-interval");
            if (!value || !parse_size(value, config.progress_interval)) {
                std::cerr << "Invalid --progress-interval value.\n";
                return false;
            }
            continue;
        }
        std::cerr << "Unknown option: " << arg << "\n";
        return false;
    }
    return true;
}

std::string iso8601_now_utc()
{
    const std::time_t now = std::time(nullptr);
    std::tm utc_time {};
#if defined(_WIN32)
    gmtime_s(&utc_time, &now);
#else
    gmtime_r(&now, &utc_time);
#endif
    std::ostringstream oss;
    oss << std::put_time(&utc_time, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

ldpc::bp::BpSparse build_pcm()
{
    ldpc::bp::BpSparse pcm(kRows, kColumns);
    std::vector<std::vector<int>> parity_check{{0, 1, 3}, {1, 2}};
    pcm.csr_insert(parity_check);
    return pcm;
}

std::unique_ptr<ldpc::bp::BpDecoder> build_decoder(ldpc::bp::BpSparse& pcm,
                                                   double ber)
{
    const double stabilized_ber = std::clamp(ber, 1e-6, 0.49);
    std::vector<double> channel_probabilities(kColumns, stabilized_ber);
    return std::make_unique<ldpc::bp::BpDecoder>(
        pcm, channel_probabilities, kMaxIterations, ldpc::bp::MINIMUM_SUM,
        ldpc::bp::PARALLEL, kBpScaling, 1, std::vector<int>{}, 0, false,
        ldpc::bp::SYNDROME);
}

bool decode_matches_error(const std::vector<std::uint8_t>& expected_error,
                          const std::vector<std::uint8_t>& decoded)
{
    if (decoded.size() != expected_error.size()) {
        return false;
    }
    for (std::size_t i = 0; i < expected_error.size(); ++i) {
        if ((decoded[i] & 1U) != (expected_error[i] & 1U)) {
            return false;
        }
    }
    return true;
}

FerResult run_single_ber(ldpc::bp::BpSparse& pcm, double ber,
                         const SimulationConfig& config, std::mt19937& rng)
{
    FerResult result;
    result.ber = ber;
    result.frames = config.trials_per_ber;

    auto decoder = build_decoder(pcm, ber);
    std::bernoulli_distribution bit_error_distribution(ber);
    std::vector<std::uint8_t> error_pattern(kColumns, 0);
    std::vector<std::uint8_t> syndrome(kRows, 0);

    const std::size_t progress_step =
        config.progress_interval == 0 ? config.trials_per_ber : config.progress_interval;

    for (std::size_t trial = 0; trial < config.trials_per_ber; ++trial) {
        for (int col = 0; col < kColumns; ++col) {
            error_pattern[static_cast<std::size_t>(col)] =
                static_cast<std::uint8_t>(bit_error_distribution(rng));
        }

        pcm.mulvec(error_pattern, syndrome);
        decoder->decode(syndrome);

        if (!decode_matches_error(error_pattern, decoder->decoding)) {
            ++result.errors;
        }

        const std::size_t done = trial + 1;
        if ((done % progress_step) == 0 || done == config.trials_per_ber) {
            std::cout << "BER=" << ber << " progress " << done << "/"
                      << config.trials_per_ber << ", frame_errors=" << result.errors
                      << "\n";
        }
    }

    result.fer = static_cast<double>(result.errors) / static_cast<double>(result.frames);
    result.standard_error =
        std::sqrt(result.fer * (1.0 - result.fer) / static_cast<double>(result.frames));
    const double ci95 = 1.96 * result.standard_error;
    result.ci95_low = std::max(0.0, result.fer - ci95);
    result.ci95_high = std::min(1.0, result.fer + ci95);
    return result;
}

bool write_csv(const std::string& output_path, const std::vector<FerResult>& results)
{
    std::ofstream csv(output_path);
    if (!csv.is_open()) {
        std::cerr << "Unable to open CSV output file: " << output_path << "\n";
        return false;
    }

    csv << "BER,FER,FramesAtBER,ErrorsAtBER,Timestamp\n";
    for (const FerResult& result : results) {
        csv << std::setprecision(10) << result.ber << "," << result.fer << ","
            << result.frames << "," << result.errors << "," << iso8601_now_utc()
            << "\n";
    }
    return true;
}

bool write_json(const std::string& output_path, const std::vector<FerResult>& results)
{
    std::ofstream json(output_path);
    if (!json.is_open()) {
        std::cerr << "Unable to open JSON output file: " << output_path << "\n";
        return false;
    }

    json << "{\n  \"generated_at\": \"" << iso8601_now_utc()
         << "\",\n  \"fer_points\": [\n";
    for (std::size_t i = 0; i < results.size(); ++i) {
        const FerResult& result = results[i];
        json << "    {\n"
             << "      \"ber\": " << std::setprecision(10) << result.ber << ",\n"
             << "      \"fer\": " << result.fer << ",\n"
             << "      \"frames\": " << result.frames << ",\n"
             << "      \"errors\": " << result.errors << ",\n"
             << "      \"standard_error\": " << result.standard_error << ",\n"
             << "      \"ci95_low\": " << result.ci95_low << ",\n"
             << "      \"ci95_high\": " << result.ci95_high << "\n"
             << "    }";
        if (i + 1 != results.size()) {
            json << ",";
        }
        json << "\n";
    }
    json << "  ]\n}\n";
    return true;
}

} // namespace

int main(int argc, char** argv)
{
    SimulationConfig config;
    if (!parse_arguments(argc, argv, config)) {
        if (argc > 1 && std::string_view(argv[1]) == "--help") {
            return 0;
        }
        print_usage(argv[0]);
        return 1;
    }

    std::cout << "Starting Monte-Carlo FER simulation with "
              << config.trials_per_ber << " trials per BER point.\n";
    std::cout << "Seed: " << config.seed << "\n";

    ldpc::bp::BpSparse pcm = build_pcm();
    std::mt19937 rng(config.seed);
    std::vector<FerResult> results;
    results.reserve(config.ber_points.size());

    const auto start = std::chrono::steady_clock::now();
    for (double ber : config.ber_points) {
        std::cout << "Running BER point " << ber << "...\n";
        const FerResult result = run_single_ber(pcm, ber, config, rng);
        std::cout << "BER=" << ber << " FER=" << result.fer
                  << " std_error=" << result.standard_error << " ci95=["
                  << result.ci95_low << ", " << result.ci95_high << "]\n";
        results.push_back(result);
    }
    const auto elapsed = std::chrono::steady_clock::now() - start;
    const double elapsed_s = std::chrono::duration<double>(elapsed).count();

    if (!write_csv(config.csv_path, results)) {
        return 1;
    }
    std::cout << "CSV results written to " << config.csv_path << "\n";

    if (config.json_path.has_value()) {
        if (!write_json(*config.json_path, results)) {
            return 1;
        }
        std::cout << "JSON results written to " << *config.json_path << "\n";
    }

    std::cout << "Monte-Carlo FER simulation completed in " << elapsed_s << " s.\n";
    return 0;
}
