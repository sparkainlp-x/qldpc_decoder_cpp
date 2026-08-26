#include <ap_int.h>
#include <hls_stream.h>

constexpr int N_QUBITS = 144;
constexpr int M_CHECKS = 66;
constexpr int MAX_BP_ITER = 20;

typedef ap_uint<1> bit_t;
typedef ap_int<6> llr_t;

struct SyndromeStream {
    ap_uint<M_CHECKS> data;
    bit_t last;
};

struct CorrectionStream {
    ap_uint<N_QUBITS> data;
    bit_t last;
};

static bit_t check_syndrome_match(
    const bit_t error_vec[N_QUBITS],
    const ap_uint<M_CHECKS>& target_syndrome)
{
#pragma HLS INLINE
    bit_t match = 1;
CHECK_LOOP:
    for (int i = 0; i < M_CHECKS; ++i) {
#pragma HLS UNROLL
        if (error_vec[i] != target_syndrome[i]) {
            match = 0;
        }
    }
    return match;
}

void qldpc_decode_kernel(
    hls::stream<SyndromeStream>& in_syndrome,
    hls::stream<CorrectionStream>& out_correction)
{
#pragma HLS INTERFACE axis register port=in_syndrome
#pragma HLS INTERFACE axis register port=out_correction
#pragma HLS INTERFACE s_axilite port=return bundle=control

    bit_t estimated_error[N_QUBITS] = {};
#pragma HLS ARRAY_PARTITION variable=estimated_error complete dim=1
    llr_t llr_mem[N_QUBITS] = {};
#pragma HLS ARRAY_PARTITION variable=llr_mem complete dim=1

    const SyndromeStream rx_syndrome = in_syndrome.read();
    ap_uint<N_QUBITS> correction_mask = 0;

BP_LOOP:
    for (int iter = 0; iter < MAX_BP_ITER; ++iter) {
#pragma HLS PIPELINE II=1
        (void)llr_mem;
        if (check_syndrome_match(estimated_error, rx_syndrome.data)) {
            break;
        }
    }

    CorrectionStream tx_correction;
    tx_correction.data = correction_mask;
    tx_correction.last = rx_syndrome.last;
    out_correction.write(tx_correction);
}
