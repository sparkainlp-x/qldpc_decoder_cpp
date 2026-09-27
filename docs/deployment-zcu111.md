# ZCU111 deployment: PetaLinux, FPGA and ARM driver (UNRUN)

> **Status: UNRUN.** This procedure has not been executed and no board results are published. It documents the intended flow only.

This procedure describes deploying the qLDPC chain to an AMD/Xilinx Zynq UltraScale+ RFSoC ZCU111 board. It assumes that the PetaLinux artifacts, the FPGA bitstream, the Device Tree overlay and the ARM driver have already been generated.

## 1. Prepare the microSD card

Create two partitions: a FAT32 `BOOT` partition of at least 1 GB with the `boot` and `lba` flags, and an ext4 `rootfs` partition with the remaining space.

Copy the boot files to the FAT32 partition:

```bash
cp BOOT.BIN image.ub boot.scr /media/$USER/BOOT/
sudo tar -xvf rootfs.tar.gz -C /media/$USER/rootfs/
sync
```

Set the ZCU111 boot switch SW6 to SD mode: switch 1 `OFF`, switch 2 `ON`, switch 3 `OFF`, switch 4 `OFF`.

## 2. Boot the board and program the FPGA

Connect the USB-UART port and open the serial console at 115200 baud:

```bash
picocom -b 115200 /dev/ttyUSB1
```

To program a bitstream dynamically with FPGA Manager, copy the bitstream and overlay to `/lib/firmware`, then run:

```bash
mkdir -p /lib/firmware
cp qldpc_decoder.bit.bin /lib/firmware/
cp qldpc_overlay.dtbo /lib/firmware/
fpgautil -b /lib/firmware/qldpc_decoder.bit.bin \
    -o /lib/firmware/qldpc_overlay.dtbo
```

The board's DONE LED should confirm that the FPGA is programmed.

## 3. Build and run the ARM driver

Copy the driver to the board:

```bash
scp axi_dma_driver.cpp root@<ZCU111_BOARD_IP>:/root/
```

Compile it for the Cortex-A53:

```bash
g++ -O3 -march=armv8-a -mcpu=cortex-a53 \
    axi_dma_driver.cpp -o axi_dma_driver
```

Check that the udmabuf buffers exist:

```bash
ls -l /dev/udmabuf*
```

Then run the driver with the required privileges:

```bash
./axi_dma_driver
```

## 4. Hardware precautions

AXI-DMA addresses, channels, interrupts, data width and physical buffer addresses must match the deployed Vivado Block Design and Device Tree exactly. Do not use the example addresses on a different board or without checking the CMA/udmabuf reservation.

The current driver uses privileged hardware access through `/dev/mem`. Do not run it on a development workstation or on a board whose memory map has not been validated. A udmabuf version should read physical addresses from `/sys/class/u-dma-buf/` and map the `/dev/udmabuf_*` devices instead of relying on hard-coded physical addresses.

GitHub Actions CI cannot run this procedure: it has no ZCU111 hardware, no Vivado/Vitis HLS, and none of the required embedded Linux devices. CI continues to validate the software tests, the micro-benchmark and source consistency.
