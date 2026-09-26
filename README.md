# qldpc_decoder_cpp

C++/HLS **research scaffold** for a qLDPC decoder: CMake + Catch2 + a sparse GF(2) micro-benchmark, plus Vitis HLS / Vivado / PetaLinux scripts targeting the AMD ZCU111 board. The BP kernel and HIL verification are placeholders; hardware results are **UNRUN**.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![C++ CI](https://github.com/sparkainlp-x/qldpc_decoder_cpp/actions/workflows/ci.yml/badge.svg)](https://github.com/sparkainlp-x/qldpc_decoder_cpp/actions/workflows/ci.yml)
[![Status: research prototype](https://img.shields.io/badge/status-research%20prototype-orange.svg)](#what-it-is-not)
[![Hardware results: UNRUN](https://img.shields.io/badge/hardware%20results-UNRUN-lightgrey.svg)](#evidence-tags)

*English first; the original French documentation follows below. / La documentation originale en français suit.*

## What it is

- A minimal C++20 program that links Joschka Roffe's [`ldpc`](https://github.com/quantumgizmos/ldpc) library through CMake `FetchContent`, with OpenMP enabled.
- A Catch2 unit-test target and a GF(2) sparse matrix-vector micro-benchmark (`ldpc_benchmark`) used as a CI regression gate.
- Scripts for an AMD hardware path on the ZCU111 RFSoC board: `hls/` (Vitis HLS kernel skeleton + testbench), `vivado/` (block design), `petalinux/`, `drivers/axi_dma_driver.cpp`, and `hil/hil_benchmark.cpp`, orchestrated by the `Makefile` and an opt-in self-hosted CI job.

## What it is NOT

- **Not** a complete qLDPC decoder on hardware. The HLS kernel (`hls/qldpc_kernel.cpp`) has no belief-propagation message updates yet.
- **Not** a quantum-hardware result. No qubits, QPU, or cryostat were used; `N_QUBITS = 144` in the HLS kernel is a code-length parameter.
- **Not** a measured decoding latency. The micro-benchmark times a single GF(2) mat-vec on a synthetic 32×64 matrix.
- **Not** hardware-ready or production software. HLS synthesis, timing closure, and HIL runs are **UNRUN**.
- `docs/scientific_review.md` is an **AI-assisted internal review**, not independent peer review.

## Quickstart

Requirements: CMake ≥ 3.20, a C++20 compiler, Git, and OpenMP (for example on Debian/Ubuntu: `sudo apt-get install -y g++ cmake libomp-dev git`). The first configure downloads `quantumgizmos/ldpc` into the build directory.

```bash
git clone https://github.com/sparkainlp-x/qldpc_decoder_cpp.git
cd qldpc_decoder_cpp
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/qldpc_decoder
```

Expected output:

```text
qldpc_decoder_cpp: bibliothèque ldpc chargée avec succès.
OpenMP est activé au niveau de la cible CMake.
```

`CMakeLists.txt` uses `-O3 -march=native -ffast-math -flto` on non-MSVC compilers, so the binary is tuned to the build machine.

## Tests

```bash
ctest --test-dir build --output-on-failure   # Catch2 unit tests
./build/ldpc_benchmark                        # GF(2) mat-vec micro-benchmark (median / p95 in ns)
```

CI (`software-ci` job in [`ci.yml`](.github/workflows/ci.yml), GitHub-hosted Ubuntu) builds with Ninja, runs the smoke binary, the Catch2 tests, and the micro-benchmark with a regression gate: it fails if the median reaches `LDPC_MAX_LATENCY_NS` (1000 ns). GitHub-hosted runners are shared VMs, so this is an indicative regression check, not a measurement. The `hardware-bitstream-build` job runs only on manual dispatch on a trusted self-hosted ZCU111 runner and has not produced published results.

## Evidence tags

| Item | Tag | Notes |
|---|---|---|
| Software build, Catch2 tests, smoke test | Runs in CI (GitHub-hosted) | |
| GF(2) sparse mat-vec micro-benchmark, ~70 ns median | **REPORTED; host/conditions unspecified** | A single micro-operation on a synthetic 32×64 matrix. **Not** qLDPC decoding latency, **not** an FPGA or end-to-end figure. CPU, compiler, OS, and raw samples were not recorded |
| HLS kernel (`hls/qldpc_kernel.cpp`) | Scaffold | No belief-propagation message updates yet |
| HLS synthesis, 300/400 MHz timing | **TARGET / UNRUN** | Clock periods are set in TCL; no post-route timing report exists |
| HIL benchmark on ZCU111, FER | **UNRUN** | Needs the board and a self-hosted runner. No run has been published |

Tag definitions: [sparkainlp-x/.github](https://github.com/sparkainlp-x/.github#evidence-tags).

### Known limitations

- **`verify_correction()` always returns `true`** (`hil/hil_benchmark.cpp`). Any HIL run would therefore report FER = 0 *by construction*. It must be replaced with a code-aware check (`H · correction = syndrome (mod 2)`, plus a logical-error check against a known injected error) before any FER figure is published.
- The HLS kernel is an integration skeleton, not a complete BP/OSD decoder.

## Relationship to ADR-001

ADR-001 governs the OES-32 residual family (normative: [oes32-residual@b77b612](https://github.com/sparkainlp-x/oes32-residual/tree/b77b61254f15778c6ae221843dceac7a8571158e); Profile A sidecars: [oes32_engine](https://github.com/sparkainlp-x/oes32_engine), [oes32-hls](https://github.com/sparkainlp-x/oes32-hls)). This decoder scaffold is **independent** of that contract: it does not compute or redefine the OES-32 residual. It shares only the ZCU111 TARGET platform with `oes32-hls`.

## Citation

Citation metadata is in [CITATION.cff](CITATION.cff); GitHub shows a "Cite this repository" button. Please also cite the upstream [`ldpc`](https://github.com/quantumgizmos/ldpc) library by Joschka Roffe.

## License

[MIT](LICENSE). Copyright (c) 2026 Jean-François Brisson, Spark AI NLP. The upstream `ldpc` library fetched at build time is distributed under its own license.

---

## Description en français

Exemple C++ minimal utilisant la bibliothèque [`ldpc`](https://github.com/quantumgizmos/ldpc) de Joschka Roffe via CMake `FetchContent`. La configuration active OpenMP ainsi que les optimisations processeur en mode non-MSVC.

### Prérequis

Il faut disposer de CMake 3.20 ou ultérieur, d’un compilateur C++ compatible C++20, de Git et d’une implémentation OpenMP.

### Compilation

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

### Exécution

```bash
./build/qldpc_decoder
```

### Benchmark de latence

Le projet construit également `ldpc_benchmark`, qui mesure une multiplication matrice-vecteur sparse sur GF(2) après une phase d’échauffement. Il exécute 31 échantillons de 1 000 itérations, puis affiche la latence médiane et le 95e percentile en nanosecondes. Les valeurs d'environ 70 ns observées jusqu'ici sont **REPORTED ; hôte/conditions non précisés** et concernent une micro-opération, pas le décodage qLDPC complet.

```bash
./build/ldpc_benchmark
```

Dans GitHub Actions, le benchmark échoue si la médiane atteint ou dépasse `1000 ns` (`1 µs`). Le seuil peut être changé avec la variable d’environnement `LDPC_MAX_LATENCY_NS`. Les runners GitHub-hosted étant virtualisés et partagés, le résultat est un contrôle de régression indicatif et ne remplace pas une mesure sur machine dédiée.

La première configuration télécharge automatiquement le dépôt amont `quantumgizmos/ldpc` dans le répertoire de build. Le fichier `CMakeLists.txt` utilise `-O3 -march=native -ffast-math -flto` sur les compilateurs non-MSVC et `/O2 /arch:AVX2` sous MSVC.

> `-march=native` produit un binaire adapté à la machine de compilation. Pour distribuer le binaire sur d’autres processeurs, remplacez ce drapeau par une architecture cible portable.

### Synthèse AMD Vitis HLS

Le répertoire `hls/` contient le noyau `qldpc_decode_kernel`, son testbench et le script `run_hls.tcl` ciblant le RFSoC ZCU111 (`xczu28dr-ffvg1517-2-e`) à 300 MHz. Depuis un environnement où Vitis HLS 2023.2 est installé :

```bash
source /tools/Xilinx/Vitis_HLS/2023.2/settings64.sh
vitis_hls -f hls/run_hls.tcl
```

La synthèse FPGA n’est pas exécutée par la CI GitHub Actions standard, car le runner ne fournit ni Vitis HLS ni les bibliothèques AMD HLS. La CI vérifie en revanche les sources C++ portables, les tests Catch2 et le benchmark logiciel.

### Pipeline matériel avec Make

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

### CI matérielle sur runner auto-hébergé

Le workflow contient un job `hardware-bitstream-build` qui s’exécute uniquement sur un runner GitHub auto-hébergé portant les labels `self-hosted`, `vivado` et `zcu111`. Ce runner doit disposer de Vitis, Vitis HLS, Vivado, PetaLinux, des licences AMD nécessaires et d’un projet PetaLinux initialisé dans `petalinux/`.

Le job matériel attend la réussite de `software-ci`, lance `make all`, puis publie les fichiers `.xsa`, `BOOT.BIN`, `image.ub` et `download.bit` comme artefacts GitHub Actions. Pour protéger la machine locale, il n’est pas déclenché par les pull requests : les changements doivent d’abord être fusionnés dans `main`, ou le workflow doit être lancé manuellement par un opérateur de confiance.

### Test HIL automatisé (UNRUN)

> **Statut : UNRUN.** Aucun test HIL n'a été exécuté ni publié. `verify_correction()` retourne toujours `true` (voir « Known limitations » ci-dessus) : tant qu'elle n'est pas remplacée, le FER rapporté vaut 0 par construction.

Le benchmark [`hil/hil_benchmark.cpp`](./hil/hil_benchmark.cpp) exécute 100 000 transferts AXI-DMA/FPGA, mesure chaque aller-retour en nanosecondes, exporte `latencies_report.csv` et vérifie la latence maximale.

Le job `hardware-bitstream-build` de GitHub Actions compile ce binaire avec `BUILD_HIL_BENCHMARK=ON`, puis l’exécute avec `HIL_MAX_LATENCY_US=100`. Le job échoue dès qu’une mesure dépasse 100 µs et conserve le CSV comme artefact `qldpc-hil-latency-report`.

Ce test ne doit pas être lancé sur `ubuntu-latest` : il nécessite `/dev/mem`, le contrôleur AXI-DMA, le bitstream chargé, les buffers udmabuf et une carte ZCU111. La vérification FER du fichier HIL est volontairement un point d’extension ; elle doit être remplacée par le calcul réel `H * correction == syndrome` et, si nécessaire, par une vérification d’erreur logique.
