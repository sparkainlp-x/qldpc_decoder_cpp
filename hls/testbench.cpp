#include "qldpc_kernel.cpp"

#include <cassert>
#include <iostream>

int main()
{
    hls::stream<SyndromeStream> input;
    hls::stream<CorrectionStream> output;

    SyndromeStream syndrome{};
    syndrome.data = 0;
    syndrome.last = 1;
    input.write(syndrome);

    qldpc_decode_kernel(input, output);

    assert(!output.empty());
    const CorrectionStream correction = output.read();
    assert(correction.last == syndrome.last);
    std::cout << "HLS C-simulation smoke test passed\n";
    return 0;
}
