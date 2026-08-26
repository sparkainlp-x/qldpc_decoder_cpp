# ================================================================================
# Pipeline matériel et embarqué qLDPC : HLS -> Vivado -> PetaLinux
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
	@echo " make hls        Synthèse Vitis HLS à 300 MHz"
	@echo " make hls-400mhz Synthèse Vitis HLS à 400 MHz"
	@echo " make vivado     Génération du Block Design et export XSA"
	@echo " make petalinux  Compilation de l’image PetaLinux"
	@echo " make all        Exécution complète HLS -> Vivado -> PetaLinux"
	@echo " make clean      Suppression des artefacts matériels"
	@echo "===================================================================="

all: hls vivado petalinux
	@echo ">>> SUCCESS : pipeline complet exécuté."

hls:
	@echo ">>> [1/3] Synthèse Vitis HLS 300 MHz..."
	$(VITIS_HLS) -f $(HLS_DIR)/run_hls.tcl

hls-400mhz:
	@echo ">>> Synthèse Vitis HLS 400 MHz..."
	$(VITIS_HLS) -f $(HLS_DIR)/run_hls_400mhz.tcl

vivado: hls
	@echo ">>> [2/3] Génération Vivado du Block Design..."
	cd $(VIVADO_DIR) && $(VIVADO) -mode batch -source create_bd.tcl

petalinux: vivado
	@echo ">>> [3/3] Compilation PetaLinux..."
	@if [ ! -d "$(PETALINUX_DIR)" ]; then \
		echo "Erreur: $(PETALINUX_DIR) est absent; initialisez d’abord un projet PetaLinux."; \
		exit 1; \
	fi
	cd $(PETALINUX_DIR) && \
	petalinux-config --get-hw-description ../$(XSA_PATH) --silent && \
	petalinux-build && \
	petalinux-package --boot \
		--fsbl images/linux/zynqmp_fsbl.elf \
		--u-boot images/linux/u-boot.elf \
		--fpga images/linux/download.bit --force
	@echo ">>> Artifact final : $(PETALINUX_DIR)/images/linux/BOOT.BIN"

clean:
	@echo ">>> Nettoyage des artefacts matériels..."
	rm -rf $(HLS_DIR)/qldpc_hls_project $(HLS_DIR)/qldpc_hls_project_400mhz
	rm -rf $(HLS_DIR)/*.log $(HLS_DIR)/vivado* $(HLS_DIR)/.Xil
	rm -rf $(VIVADO_DIR)/qldpc_vivado_bd $(VIVADO_DIR)/*.jou $(VIVADO_DIR)/*.log
	rm -rf $(VIVADO_DIR)/*.xsa $(VIVADO_DIR)/.Xil
	@echo ">>> Nettoyage terminé."
