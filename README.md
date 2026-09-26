# qldpc_decoder_cpp

[![C++ CI](https://github.com/sparkainlp-x/qldpc_decoder_cpp/actions/workflows/ci.yml/badge.svg)](https://github.com/sparkainlp-x/qldpc_decoder_cpp/actions/workflows/ci.yml)

## English summary

A C++/HLS **research scaffold** for a qLDPC decoder: CMake + Catch2 + a sparse GF(2) micro-benchmark, plus Vitis HLS / Vivado / PetaLinux scripts that target the AMD ZCU111 board. It uses Joschka Roffe's [`ldpc`](https://github.com/quantumgizmos/ldpc) library through CMake `FetchContent`. This is research software, not a hardware product and not a quantum-hardware result.

### Evidence status

| Item | Tag | Notes |
|---|---|---|
| Software build, Catch2 tests, smoke test | Runs in CI (GitHub-hosted) | |
| GF(2) sparse mat-vec micro-benchmark, ~70 ns median | **REPORTED; host/conditions unspecified** | A single micro-operation on a synthetic 32×64 matrix. **Not** qLDPC decoding latency, **not** an FPGA or end-to-end figure. CPU, compiler, OS, and raw samples were not recorded |
| HLS kernel (`hls/qldpc_kernel.cpp`) | Scaffold | No belief-propagation message updates yet |
| HLS synthesis, 300/400 MHz timing | **TARGET / UNRUN** | Clock periods are set in TCL; no post-route timing report exists |
| HIL benchmark on ZCU111, FER | **UNRUN** | Needs the board and a self-hosted runner. No run has been published |

### Known limitations

- **`verify_correction()` always returns `true`** (`hil/hil_benchmark.cpp`). Any HIL run would therefore report FER = 0 *by construction*. It must be replaced with a code-aware check (`H · correction = syndrome (mod 2)`, plus a logical-error check against a known injected error) before any FER figure is published.
- The HLS kernel is an integration skeleton, not a complete BP/OSD decoder.
- `docs/scientific_review.md` is an **AI-assisted internal review**, not independent peer review.

A French description follows. / La description en français suit.

---

## Description (français)

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

Le projet construit également `ldpc_benchmark`, qui mesure une multiplication matrice-vecteur sparse sur GF(2) après une phase d’échauffement. Il exécute 31 échantillons de 1 000 itérations, puis affiche la latence médiane et le 95e percentile en nanosecondes. Les valeurs d'environ 70 ns observées jusqu'ici sont **REPORTED ; hôte/conditions non précisés** et concernent une micro-opération, pas le décodage qLDPC complet.

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

## Test HIL automatisé (UNRUN)

> **Statut : UNRUN.** Aucun test HIL n'a été exécuté ni publié. `verify_correction()` retourne toujours `true` (voir « Known limitations » ci-dessus) : tant qu'elle n'est pas remplacée, le FER rapporté vaut 0 par construction.

Le benchmark [`hil/hil_benchmark.cpp`](./hil/hil_benchmark.cpp) exécute 100 000 transferts AXI-DMA/FPGA, mesure chaque aller-retour en nanosecondes, exporte `latencies_report.csv` et vérifie la latence maximale.

Le job `hardware-bitstream-build` de GitHub Actions compile ce binaire avec `BUILD_HIL_BENCHMARK=ON`, puis l’exécute avec `HIL_MAX_LATENCY_US=100`. Le job échoue dès qu’une mesure dépasse 100 µs et conserve le CSV comme artefact `qldpc-hil-latency-report`.

Ce test ne doit pas être lancé sur `ubuntu-latest` : il nécessite `/dev/mem`, le contrôleur AXI-DMA, le bitstream chargé, les buffers udmabuf et une carte ZCU111. La vérification FER du fichier HIL est volontairement un point d’extension ; elle doit être remplacée par le calcul réel `H * correction == syndrome` et, si nécessaire, par une vérification d’erreur logique.
