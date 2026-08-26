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

option(BUILD_UNIT_TESTS "Build Catch2 unit tests" ON)
if(BUILD_UNIT_TESTS)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.2
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(Catch2)
    enable_testing()
endif()

add_executable(qldpc_decoder main.cpp)
add_executable(ldpc_benchmark benchmark.cpp)

if(BUILD_UNIT_TESTS)
    add_executable(unit_tests tests/test_decoder.cpp)
    target_link_libraries(unit_tests PRIVATE Catch2::Catch2WithMain ldpc OpenMP::OpenMP_CXX)
    target_include_directories(unit_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    add_test(NAME unit_tests COMMAND unit_tests)
endif()

target_link_libraries(qldpc_decoder
    PRIVATE
        ldpc
        OpenMP::OpenMP_CXX
)

target_link_libraries(ldpc_benchmark
    PRIVATE
        ldpc
        OpenMP::OpenMP_CXX
)

target_include_directories(qldpc_decoder PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(ldpc_benchmark PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(NOT MSVC)
    set_property(TARGET qldpc_decoder PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    set_property(TARGET ldpc_benchmark PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    if(BUILD_UNIT_TESTS)
        set_property(TARGET unit_tests PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endif()
