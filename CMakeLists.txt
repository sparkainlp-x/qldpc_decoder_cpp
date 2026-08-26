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
option(BUILD_AXI_DMA_DRIVER "Build the Linux ARM AXI-DMA driver" OFF)
option(BUILD_HIL_BENCHMARK "Build the hardware-in-the-loop benchmark" OFF)
option(BUILD_MONTE_CARLO_SIM "Build the Monte-Carlo FER simulator" ON)
if(BUILD_UNIT_TESTS OR BUILD_MONTE_CARLO_SIM)
    enable_testing()
endif()
if(BUILD_AXI_DMA_DRIVER)
    if(NOT UNIX)
        message(FATAL_ERROR "BUILD_AXI_DMA_DRIVER requires a Unix-like target")
    endif()
    add_executable(axi_dma_driver drivers/axi_dma_driver.cpp)
endif()
if(BUILD_HIL_BENCHMARK)
    if(NOT UNIX)
        message(FATAL_ERROR "BUILD_HIL_BENCHMARK requires a Unix-like target")
    endif()
    add_executable(hil_benchmark hil/hil_benchmark.cpp)
endif()
if(BUILD_UNIT_TESTS)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.2
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(Catch2)
endif()

add_executable(qldpc_decoder main.cpp)
add_executable(ldpc_benchmark benchmark.cpp)
if(BUILD_MONTE_CARLO_SIM)
    add_executable(monte_carlo_sim monte_carlo.cpp)
endif()

if(BUILD_UNIT_TESTS)
    add_executable(unit_tests tests/test_decoder.cpp)
    target_link_libraries(unit_tests PRIVATE Catch2::Catch2WithMain ldpc OpenMP::OpenMP_CXX)
    target_include_directories(unit_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    add_test(NAME unit_tests COMMAND unit_tests)
endif()

if(BUILD_MONTE_CARLO_SIM)
    add_test(
        NAME monte_carlo_smoke
        COMMAND monte_carlo_sim
            --trials 50
            --ber-list 0.01
            --csv ${CMAKE_BINARY_DIR}/fer_curve_test.csv
            --progress-interval 50
    )
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

if(BUILD_MONTE_CARLO_SIM)
    target_link_libraries(monte_carlo_sim
        PRIVATE
            ldpc
            OpenMP::OpenMP_CXX
    )
endif()

target_include_directories(qldpc_decoder PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(ldpc_benchmark PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
if(BUILD_MONTE_CARLO_SIM)
    target_include_directories(monte_carlo_sim PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
endif()

if(NOT MSVC)
    set_property(TARGET qldpc_decoder PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    set_property(TARGET ldpc_benchmark PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    if(BUILD_MONTE_CARLO_SIM)
        set_property(TARGET monte_carlo_sim PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
    if(BUILD_UNIT_TESTS)
        set_property(TARGET unit_tests PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
    if(BUILD_AXI_DMA_DRIVER)
        set_property(TARGET axi_dma_driver PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
    if(BUILD_HIL_BENCHMARK)
        set_property(TARGET hil_benchmark PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endif()
