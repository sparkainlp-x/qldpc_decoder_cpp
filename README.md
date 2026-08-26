# qldpc_decoder_cpp

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

La première configuration télécharge automatiquement le dépôt amont `quantumgizmos/ldpc` dans le répertoire de build. Le fichier `CMakeLists.txt` utilise `-O3 -march=native -ffast-math -flto` sur les compilateurs non-MSVC et `/O2 /arch:AVX2` sous MSVC.

> `-march=native` produit un binaire adapté à la machine de compilation. Pour distribuer le binaire sur d’autres processeurs, remplacez ce drapeau par une architecture cible portable.
