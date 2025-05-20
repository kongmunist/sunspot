# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/Users/andykong/Library/Mobile Documents/com~apple~CloudDocs/hardware/sunspot/firmware/rp2040_ov2640/pico-sdk/tools/elf2uf2"
  "/Users/andykong/Library/Mobile Documents/com~apple~CloudDocs/hardware/sunspot/firmware/build/elf2uf2"
  "/Users/andykong/Library/Mobile Documents/com~apple~CloudDocs/hardware/sunspot/firmware/build/elf2uf2"
  "/Users/andykong/Library/Mobile Documents/com~apple~CloudDocs/hardware/sunspot/firmware/build/elf2uf2/tmp"
  "/Users/andykong/Library/Mobile Documents/com~apple~CloudDocs/hardware/sunspot/firmware/build/elf2uf2/src/ELF2UF2Build-stamp"
  "/Users/andykong/Library/Mobile Documents/com~apple~CloudDocs/hardware/sunspot/firmware/build/elf2uf2/src"
  "/Users/andykong/Library/Mobile Documents/com~apple~CloudDocs/hardware/sunspot/firmware/build/elf2uf2/src/ELF2UF2Build-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/andykong/Library/Mobile Documents/com~apple~CloudDocs/hardware/sunspot/firmware/build/elf2uf2/src/ELF2UF2Build-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/andykong/Library/Mobile Documents/com~apple~CloudDocs/hardware/sunspot/firmware/build/elf2uf2/src/ELF2UF2Build-stamp${cfgdir}") # cfgdir has leading slash
endif()
