cmake_minimum_required(VERSION 3.22.1)

project(6502-cxx CXX ASM)

if (NOT LLVM_MOS_PLATFORM)
  message(FATAL_ERROR "LLVM_MOS_PLATFORM must be set.")
endif()

string(TOUPPER ${LLVM_MOS_PLATFORM} LLVM_MOS_PLATFORM)
string(MAKE_C_IDENTIFIER ${LLVM_MOS_PLATFORM} LLVM_MOS_PLATFORM)

message(STATUS "Building for LLVM_MOS_PLATFORM ${LLVM_MOS_PLATFORM}")

if (LLVM_MOS_PLATFORM STREQUAL C64)
  add_subdirectory(c64lib)
elseif (LLVM_MOS_PLATFORM MATCHES NES)
  add_subdirectory(neslib)
endif()

set_property(SOURCE minesweeper-gfx.S APPEND PROPERTY INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR})

set(TARGET_SOURCES_C64 target_c64.h minesweeper-gfx.S)
set(NES_CHR_ROM ${CMAKE_CURRENT_SOURCE_DIR}/mossweeper.chr)

set(RESOURCE_COMPILE_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/cmake/MakeResource.cmake)
add_custom_command(OUTPUT minesweeper.chr.inc
  COMMAND ${CMAKE_COMMAND}
  ARGS
    -DINPUT=${NES_CHR_ROM}
    -DOUTPUT=minesweeper.chr.inc
    -DSYMBOL_NAME=tile_data
    -P ${RESOURCE_COMPILE_SCRIPT}
  DEPENDS ${RESOURCE_COMPILE_SCRIPT} ${NES_CHR_ROM}
)

set(TARGET_SOURCES_NES_NROM_128
  target_nes.h 
  minesweeper-chr_rom.S
)

add_executable(minesweeper
  minesweeper.cpp
  rand.h rand.cpp
  tile_model.h
  platform_switch.h
  ${TARGET_SOURCES_${LLVM_MOS_PLATFORM}}
)

set_property(SOURCE minesweeper-gfx.S
  APPEND PROPERTY OBJECT_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/minesweeper-charset.s
  ${CMAKE_CURRENT_SOURCE_DIR}/minesweeper-colors.S
  ${CMAKE_CURRENT_SOURCE_DIR}/cursor.S
  ${CMAKE_CURRENT_SOURCE_DIR}/bg-sprites.S
)

set_property(SOURCE minesweeper-chr_rom.S
  APPEND PROPERTY OBJECT_DEPENDS
  minesweeper.chr.inc
)

set_property(TARGET minesweeper APPEND PROPERTY COMPILE_DEFINITIONS PLATFORM_${LLVM_MOS_PLATFORM})

set(TARGET_SUFFIX_C64 .prg)
set(TARGET_SUFFIX_NES_NROM_128 .nes)
set_property(TARGET minesweeper PROPERTY SUFFIX ${TARGET_SUFFIX_${LLVM_MOS_PLATFORM}})

set(TARGET_LINK_C64 c64lib)
set(TARGET_LINK_NES_NROM_128 neslib)
target_link_libraries(minesweeper ${TARGET_LINK_${LLVM_MOS_PLATFORM}})

set_property(TARGET minesweeper PROPERTY CXX_STANDARD 17)
set_property(TARGET minesweeper PROPERTY LINK_FLAGS -Wl,-Map=output.map)

