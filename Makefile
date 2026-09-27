# ================================================================================
# qLDPC hardware and embedded pipeline: HLS -> Vivado -> PetaLinux (UNRUN)
# ================================================================================

HLS_DIR       := hls
VIVADO_DIR    := vivado
PETALINUX_DIR := petalinux

VITIS_HLS ?= vitis_hls
VIVADO    ?= vivado

XSA_NAME := qldpc_rfsoc_system.xsa
XSA_PATH := $(VIVADO_DIR)/$(XSA_NAME)

.PHONY: all help hls hls-400mhz vivado petalinux clean

help:
	@echo "===================================================================="
	@echo " Makefile qLDPC - Pipeline ZCU111 RFSoC"
	@echo "===================================================================="
	@echo " make hls        Vitis HLS synthesis at 300 MHz (TARGET)"
	@echo " make hls-400mhz Vitis HLS synthesis at 400 MHz (TARGET)"
	@echo " make vivado     Generate Block Design and export XSA"
	@echo " make petalinux  Build the PetaLinux image"
	@echo " make all        Full run HLS -> Vivado -> PetaLinux"
	@echo " make clean      Remove hardware build artifacts"
	@echo "===================================================================="

all: hls vivado petalinux
	@echo ">>> Pipeline finished."

hls:
	@echo ">>> [1/3] Vitis HLS synthesis, 300 MHz..."
	$(VITIS_HLS) -f $(HLS_DIR)/run_hls.tcl

hls-400mhz:
	@echo ">>> Vitis HLS synthesis, 400 MHz..."
	$(VITIS_HLS) -f $(HLS_DIR)/run_hls_400mhz.tcl

vivado: hls
	@echo ">>> [2/3] Generating Vivado Block Design..."
	cd $(VIVADO_DIR) && $(VIVADO) -mode batch -source create_bd.tcl

petalinux: vivado
	@echo ">>> [3/3] Building PetaLinux..."
	@if [ ! -d "$(PETALINUX_DIR)" ]; then \
		echo "Error: $(PETALINUX_DIR) is missing; initialise a PetaLinux project first."; \
		exit 1; \
	fi
	cd $(PETALINUX_DIR) && \
	petalinux-config --get-hw-description ../$(XSA_PATH) --silent && \
	petalinux-build && \
	petalinux-package --boot \
		--fsbl images/linux/zynqmp_fsbl.elf \
		--u-boot images/linux/u-boot.elf \
		--fpga images/linux/download.bit --force
	@echo ">>> Final artifact: $(PETALINUX_DIR)/images/linux/BOOT.BIN"

clean:
	@echo ">>> Removing hardware build artifacts..."
	rm -rf $(HLS_DIR)/qldpc_hls_project $(HLS_DIR)/qldpc_hls_project_400mhz
	rm -rf $(HLS_DIR)/*.log $(HLS_DIR)/vivado* $(HLS_DIR)/.Xil
	rm -rf $(VIVADO_DIR)/qldpc_vivado_bd $(VIVADO_DIR)/*.jou $(VIVADO_DIR)/*.log
	rm -rf $(VIVADO_DIR)/*.xsa $(VIVADO_DIR)/.Xil
	@echo ">>> Clean finished."
