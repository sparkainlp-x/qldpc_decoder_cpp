cmake_minimum_required(VERSION 3.20)
project(qldpc_decoder_cpp LANGUAGES CXX)

# La version actuelle de ldpc utilise des fonctionnalités C++20.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Optimisations adaptées à une compilation Release sur la machine hôte.
if(MSVC)
    add_compile_options(/O2 /arch:AVX2)
else()
    add_compile_options(-O3 -march=native -ffast-math -flto)
endif()

# OpenMP est requis pour les chemins parallélisés de l’application.
find_package(OpenMP REQUIRED)

# Bibliothèque C++ ldpc de Joschka Roffe.
include(FetchContent)
FetchContent_Declare(
    ldpc
    GIT_REPOSITORY https://github.com/quantumgizmos/ldpc.git
    GIT_TAG        main
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(ldpc)

add_executable(qldpc_decoder main.cpp)

target_link_libraries(qldpc_decoder
    PRIVATE
        ldpc
        OpenMP::OpenMP_CXX
)

target_include_directories(qldpc_decoder PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(NOT MSVC)
    set_property(TARGET qldpc_decoder PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()
