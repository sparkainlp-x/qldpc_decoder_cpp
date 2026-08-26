# qldpc_decoder_cpp

[![C++ CI](https://github.com/sparkainlp-x/qldpc_decoder_cpp/actions/workflows/ci.yml/badge.svg)](https://github.com/sparkainlp-x/qldpc_decoder_cpp/actions/workflows/ci.yml)

Exemple C++ minimal utilisant la bibliothèque [`ldpc`](https://github.com/quantumgizmos/ldpc) de Joschka Roffe via CMake `FetchContent`. La configuration active OpenMP ainsi que les optimisations processeur en mode non-MSVC.

## Prérequis

Il faut disposer de CMake 3.20 ou ultérieur, d’un compilateur C++ compatible C++20, de Git et d’une implémentation OpenMP.

## Compilation

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

## Exécution

```bash
./build/qldpc_decoder
```

## Benchmark de latence

Le projet construit également `ldpc_benchmark`, qui mesure une multiplication matrice-vecteur sparse sur GF(2) après une phase d’échauffement. Il exécute 31 échantillons de 1 000 itérations, puis affiche la latence médiane et le 95e percentile en nanosecondes.

```bash
./build/ldpc_benchmark
```

Dans GitHub Actions, le benchmark échoue si la médiane atteint ou dépasse `1000 ns` (`1 µs`). Le seuil peut être changé avec la variable d’environnement `LDPC_MAX_LATENCY_NS`. Les runners GitHub-hosted étant virtualisés et partagés, le résultat est un contrôle de régression indicatif et ne remplace pas une mesure sur machine dédiée.

La première configuration télécharge automatiquement le dépôt amont `quantumgizmos/ldpc` dans le répertoire de build. Le fichier `CMakeLists.txt` utilise `-O3 -march=native -ffast-math -flto` sur les compilateurs non-MSVC et `/O2 /arch:AVX2` sous MSVC.

> `-march=native` produit un binaire adapté à la machine de compilation. Pour distribuer le binaire sur d’autres processeurs, remplacez ce drapeau par une architecture cible portable.

## Synthèse AMD Vitis HLS

Le répertoire `hls/` contient le noyau `qldpc_decode_kernel`, son testbench et le script `run_hls.tcl` ciblant le RFSoC ZCU111 (`xczu28dr-ffvg1517-2-e`) à 300 MHz. Depuis un environnement où Vitis HLS 2023.2 est installé :

```bash
source /tools/Xilinx/Vitis_HLS/2023.2/settings64.sh
vitis_hls -f hls/run_hls.tcl
```

La synthèse FPGA n’est pas exécutée par la CI GitHub Actions standard, car le runner ne fournit ni Vitis HLS ni les bibliothèques AMD HLS. La CI vérifie en revanche les sources C++ portables, les tests Catch2 et le benchmark logiciel.

## Pipeline matériel avec Make

Le [`Makefile`](./Makefile) orchestre les étapes matérielles lorsqu’un environnement AMD est installé :

```bash
make help
make hls          # solution HLS 300 MHz
make hls-400mhz   # solution HLS 400 MHz
make vivado       # Block Design et export XSA
make petalinux    # image Linux et BOOT.BIN
make all          # chaîne complète
```

Avant son utilisation, charger les environnements Vitis/Vivado et PetaLinux correspondant à l’installation locale. Le dépôt fournit uniquement les sources et scripts ; il ne contient pas un projet PetaLinux initialisé ni les outils AMD.

## CI matérielle sur runner auto-hébergé

Le workflow contient un job `hardware-bitstream-build` qui s’exécute uniquement sur un runner GitHub auto-hébergé portant les labels `self-hosted`, `vivado` et `zcu111`. Ce runner doit disposer de Vitis, Vitis HLS, Vivado, PetaLinux, des licences AMD nécessaires et d’un projet PetaLinux initialisé dans `petalinux/`.

Le job matériel attend la réussite de `software-ci`, lance `make all`, puis publie les fichiers `.xsa`, `BOOT.BIN`, `image.ub` et `download.bit` comme artefacts GitHub Actions. Pour protéger la machine locale, il n’est pas déclenché par les pull requests : les changements doivent d’abord être fusionnés dans `main`, ou le workflow doit être lancé manuellement par un opérateur de confiance.
