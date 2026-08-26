# Déploiement ZCU111 : PetaLinux, FPGA et driver ARM

Cette procédure décrit le déploiement de la chaîne qLDPC sur une carte AMD/Xilinx Zynq UltraScale+ RFSoC ZCU111. Elle suppose que les artefacts PetaLinux, le bitstream FPGA, l’overlay Device Tree et le driver ARM ont déjà été générés.

## 1. Préparer la carte MicroSD

Créer deux partitions : une partition `BOOT` en FAT32 d’au moins 1 Go avec les drapeaux `boot` et `lba`, puis une partition `rootfs` en ext4 avec l’espace restant.

Copier les fichiers de démarrage dans la partition FAT32 :

```bash
cp BOOT.BIN image.ub boot.scr /media/$USER/BOOT/
sudo tar -xvf rootfs.tar.gz -C /media/$USER/rootfs/
sync
```

Configurer le commutateur de démarrage SW6 de la ZCU111 en mode SD : switch 1 `OFF`, switch 2 `ON`, switch 3 `OFF`, switch 4 `OFF`.

## 2. Démarrer la carte et programmer le FPGA

Connecter le port USB-UART, puis ouvrir la console série à 115200 bauds :

```bash
picocom -b 115200 /dev/ttyUSB1
```

Pour programmer un bitstream dynamiquement avec FPGA Manager, copier le bitstream et l’overlay dans `/lib/firmware`, puis exécuter :

```bash
mkdir -p /lib/firmware
cp qldpc_decoder.bit.bin /lib/firmware/
cp qldpc_overlay.dtbo /lib/firmware/
fpgautil -b /lib/firmware/qldpc_decoder.bit.bin \
    -o /lib/firmware/qldpc_overlay.dtbo
```

La LED DONE de la carte doit confirmer la programmation du FPGA.

## 3. Compiler et exécuter le driver ARM

Transférer le driver sur la carte :

```bash
scp axi_dma_driver.cpp root@<IP_CARTE_ZCU111>:/root/
```

Le compiler pour le Cortex-A53 :

```bash
g++ -O3 -march=armv8-a -mcpu=cortex-a53 \
    axi_dma_driver.cpp -o axi_dma_driver
```

Vérifier la présence des buffers udmabuf :

```bash
ls -l /dev/udmabuf*
```

Lancer ensuite le driver avec les privilèges nécessaires :

```bash
./axi_dma_driver
```

## 4. Précautions matérielles

Les adresses AXI-DMA, les canaux, les interruptions, la largeur de données et les adresses physiques des buffers doivent correspondre exactement au Block Design Vivado et au Device Tree déployés. Ne pas utiliser les adresses d’exemple sur une carte différente ou sans vérifier la réservation CMA/udmabuf.

Le driver utilise un accès matériel privilégié à `/dev/mem` dans sa version actuelle. Il ne doit pas être exécuté sur un poste de développement ou sur une carte dont la cartographie mémoire n’a pas été validée. La version avec udmabuf doit récupérer les adresses physiques depuis `/sys/class/u-dma-buf/` et mapper les périphériques `/dev/udmabuf_*` plutôt que de dépendre d’adresses physiques codées en dur.

La CI GitHub Actions ne peut pas exécuter cette procédure : elle ne dispose ni du matériel ZCU111, ni de Vivado/Vitis HLS, ni des périphériques Linux embarqués nécessaires. Elle continue de valider les tests logiciels, le benchmark et la cohérence des sources.
