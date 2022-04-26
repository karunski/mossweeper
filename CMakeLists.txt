cmake_minimum_required(VERSION 3.22.1)

project(6502-cxx CXX ASM)

add_subdirectory(c64lib)

set_property(SOURCE minesweeper-gfx.S APPEND PROPERTY INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(minesweeper
  minesweeper.cpp
  minesweeper-gfx.S
  rand.h rand.cpp
)

set_property(SOURCE minesweeper-gfx.S
  APPEND PROPERTY OBJECT_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/minesweeper-charset.s
  ${CMAKE_CURRENT_SOURCE_DIR}/minesweeper-colors.S
  ${CMAKE_CURRENT_SOURCE_DIR}/cursor.S
  ${CMAKE_CURRENT_SOURCE_DIR}/bg-sprites.S
)

target_link_libraries(minesweeper c64lib)
set_property(TARGET minesweeper PROPERTY CXX_STANDARD 17)
