#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fcntl.h>
#include <iostream>
#include <sys/mman.h>
#include <unistd.h>

// Adjust to the address actually assigned in the Vivado Block Design.
constexpr uintptr_t AXI_DMA_BASE_ADDR = 0xA0000000;
constexpr size_t AXI_DMA_MAP_SIZE = 0x10000;

constexpr uint32_t MM2S_DMACR = 0x00;
constexpr uint32_t MM2S_DMASR = 0x04;
constexpr uint32_t MM2S_SA = 0x18;
constexpr uint32_t MM2S_LENGTH = 0x28;

constexpr uint32_t S2MM_DMACR = 0x30;
constexpr uint32_t S2MM_DMASR = 0x34;
constexpr uint32_t S2MM_DA = 0x48;
constexpr uint32_t S2MM_LENGTH = 0x58;

constexpr uint32_t DMA_CR_RUNSTOP = 0x0001;
constexpr uint32_t DMA_SR_IDLE = 0x0002;

constexpr size_t SYNDROME_BYTES = 16;
constexpr size_t CORRECTION_BYTES = 32;
constexpr size_t BUFFER_MAP_SIZE = 0x1000;

class AxiDmaDriver {
private:
    int dev_mem_fd_ = -1;
    volatile uint32_t* dma_regs_ = nullptr;
    uint8_t* tx_virt_buf_ = nullptr;
    uint8_t* rx_virt_buf_ = nullptr;

    // These addresses must match a reserved CMA/udmabuf region.
    uintptr_t tx_phys_addr_ = 0x10000000;
    uintptr_t rx_phys_addr_ = 0x10020000;

    void write_reg(uint32_t offset, uint32_t value) { dma_regs_[offset / 4] = value; }
    uint32_t read_reg(uint32_t offset) { return dma_regs_[offset / 4]; }

public:
    bool init()
    {
        dev_mem_fd_ = open("/dev/mem", O_RDWR | O_SYNC);
        if (dev_mem_fd_ < 0) {
            std::cerr << "Erreur: impossible d’ouvrir /dev/mem (droits root requis)\n";
            return false;
        }

        void* mapped = mmap(nullptr, AXI_DMA_MAP_SIZE, PROT_READ | PROT_WRITE,
                            MAP_SHARED, dev_mem_fd_, AXI_DMA_BASE_ADDR);
        if (mapped == MAP_FAILED) {
            std::cerr << "Error: failed to map AXI-DMA registers\n";
            return false;
        }
        dma_regs_ = reinterpret_cast<volatile uint32_t*>(mapped);

        tx_virt_buf_ = reinterpret_cast<uint8_t*>(mmap(
            nullptr, BUFFER_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
            dev_mem_fd_, tx_phys_addr_));
        rx_virt_buf_ = reinterpret_cast<uint8_t*>(mmap(
            nullptr, BUFFER_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
            dev_mem_fd_, rx_phys_addr_));

        if (tx_virt_buf_ == MAP_FAILED || rx_virt_buf_ == MAP_FAILED) {
            std::cerr << "Error: failed to map DMA buffers\n";
            return false;
        }

        write_reg(MM2S_DMACR, DMA_CR_RUNSTOP);
        write_reg(S2MM_DMACR, DMA_CR_RUNSTOP);
        return true;
    }

    ~AxiDmaDriver()
    {
        if (dma_regs_) munmap(const_cast<uint32_t*>(dma_regs_), AXI_DMA_MAP_SIZE);
        if (tx_virt_buf_ && tx_virt_buf_ != MAP_FAILED) munmap(tx_virt_buf_, BUFFER_MAP_SIZE);
        if (rx_virt_buf_ && rx_virt_buf_ != MAP_FAILED) munmap(rx_virt_buf_, BUFFER_MAP_SIZE);
        if (dev_mem_fd_ >= 0) close(dev_mem_fd_);
    }

    void execute_decoding(const uint8_t* syndrome_in, uint8_t* correction_out)
    {
        std::copy(syndrome_in, syndrome_in + SYNDROME_BYTES, tx_virt_buf_);
        const auto start = std::chrono::steady_clock::now();

        write_reg(S2MM_DA, static_cast<uint32_t>(rx_phys_addr_));
        write_reg(S2MM_LENGTH, CORRECTION_BYTES);
        write_reg(MM2S_SA, static_cast<uint32_t>(tx_phys_addr_));
        write_reg(MM2S_LENGTH, SYNDROME_BYTES);

        while (!(read_reg(S2MM_DMASR) & DMA_SR_IDLE)) {
#if defined(__aarch64__)
            asm volatile("yield");
#endif
        }

        const auto end = std::chrono::steady_clock::now();
        const auto elapsed_us =
            std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        std::copy(rx_virt_buf_, rx_virt_buf_ + CORRECTION_BYTES, correction_out);
#if !defined(AXI_DMA_DRIVER_QUIET)
        std::cout << "[ARM Host] FPGA decode took: " << elapsed_us << " us\n";
#endif
    }
};

#if !defined(AXI_DMA_DRIVER_NO_MAIN)
int main()
{
    AxiDmaDriver dma;
    if (!dma.init()) return 1;

    uint8_t syndrome[SYNDROME_BYTES] = {0x05, 0x80, 0x12, 0x00};
    uint8_t correction[CORRECTION_BYTES] = {0};
    dma.execute_decoding(syndrome, correction);
    return 0;
}
#endif
