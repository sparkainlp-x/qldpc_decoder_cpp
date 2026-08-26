# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/ubuntu/qldpc_project/build-hil/_deps/ldpc-src"
  "/home/ubuntu/qldpc_project/build-hil/_deps/ldpc-build"
  "/home/ubuntu/qldpc_project/build-hil/_deps/ldpc-subbuild/ldpc-populate-prefix"
  "/home/ubuntu/qldpc_project/build-hil/_deps/ldpc-subbuild/ldpc-populate-prefix/tmp"
  "/home/ubuntu/qldpc_project/build-hil/_deps/ldpc-subbuild/ldpc-populate-prefix/src/ldpc-populate-stamp"
  "/home/ubuntu/qldpc_project/build-hil/_deps/ldpc-subbuild/ldpc-populate-prefix/src"
  "/home/ubuntu/qldpc_project/build-hil/_deps/ldpc-subbuild/ldpc-populate-prefix/src/ldpc-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/ubuntu/qldpc_project/build-hil/_deps/ldpc-subbuild/ldpc-populate-prefix/src/ldpc-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/ubuntu/qldpc_project/build-hil/_deps/ldpc-subbuild/ldpc-populate-prefix/src/ldpc-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
