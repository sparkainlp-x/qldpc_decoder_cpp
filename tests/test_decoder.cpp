#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <cstdint>
#include <memory>
#include <vector>

#include "bp.hpp"

namespace {

struct DecoderFixture {
    ldpc::bp::BpSparse pcm{2, 4};
    std::vector<double> channel_probabilities{0.01, 0.01, 0.01, 0.01};
    std::unique_ptr<ldpc::bp::BpDecoder> decoder;

    DecoderFixture()
    {
        std::vector<std::vector<int>> parity_check{{0, 1, 3}, {1, 2}};
        pcm.csr_insert(parity_check);
        decoder = std::make_unique<ldpc::bp::BpDecoder>(
            pcm, channel_probabilities, 40, ldpc::bp::MINIMUM_SUM,
            ldpc::bp::PARALLEL, 0.625, 1, std::vector<int>{}, 0, false,
            ldpc::bp::SYNDROME);
    }
};

} // namespace

TEST_CASE("BP decoder returns a correction", "[decoder]")
{
    DecoderFixture fixture;
    std::vector<std::uint8_t> syndrome{1, 0};

    REQUIRE_NOTHROW(fixture.decoder->decode(syndrome));
    REQUIRE(fixture.decoder->decoding.size() == 4);
}

TEST_CASE("BP decoder latency stays below 100 microseconds", "[latency]")
{
    DecoderFixture fixture;
    std::vector<std::uint8_t> syndrome{1, 0};

    for (int i = 0; i < 10; ++i) {
        fixture.decoder->decode(syndrome);
    }

    const auto start = std::chrono::steady_clock::now();
    fixture.decoder->decode(syndrome);
    const auto elapsed = std::chrono::steady_clock::now() - start;
    const auto elapsed_us =
        std::chrono::duration<double, std::micro>(elapsed).count();

    INFO("BP decode latency: " << elapsed_us << " us");
    REQUIRE(elapsed_us < 100.0);
}
