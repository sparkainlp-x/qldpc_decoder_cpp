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
