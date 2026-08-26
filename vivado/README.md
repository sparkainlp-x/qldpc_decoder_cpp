# Block Design Vivado RFSoC

Le script [`create_bd.tcl`](./create_bd.tcl) génère le Block Design `system_bd` pour un AMD Zynq UltraScale+ RFSoC ZCU111 (`xczu28dr-ffvg1517-2-e`). Il ajoute le dépôt IP produit par Vitis HLS, instancie le processeur RFSoC, le convertisseur RF, les FIFOs AXI4-Stream et le noyau `qldpc_decode_kernel`.

Le script suppose que le projet Vitis HLS a déjà été généré avec l’IP disponible à l’emplacement suivant :

```text
./qldpc_hls_project/solution_300mhz/impl/ip
```

Depuis un environnement AMD Vivado correctement configuré, lancer :

```bash
vivado -mode batch -source vivado/create_bd.tcl
```

Le script exécute ensuite `validate_bd_design` et sauvegarde le Block Design. La génération dépend des versions installées des IP AMD/Xilinx et ne peut pas être validée dans la CI logicielle standard, qui ne fournit pas Vivado ni les licences FPGA nécessaires.

## Variante 400 MHz

La variante [`../hls/run_hls_400mhz.tcl`](../hls/run_hls_400mhz.tcl) crée une solution `solution_400mhz` avec une période cible de `2.500 ns`. Le script [`optimize_timing_400mhz.tcl`](./optimize_timing_400mhz.tcl) active le retiming, les stratégies d’implémentation orientées performance et génère `timing_400mhz_report.txt`.

```bash
vitis_hls -f hls/run_hls_400mhz.tcl
vivado -mode batch -source vivado/optimize_timing_400mhz.tcl
```

La cible 400 MHz n’est atteinte que si le rapport post-routage confirme un WNS supérieur ou égal à `0 ns`. Les directives d’optimisation ne constituent pas une garantie de fréquence : la validation dépend du placement-routage réel, de la version des outils, des contraintes d’horloge et de la configuration exacte du Block Design.
