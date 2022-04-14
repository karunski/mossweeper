add_library(c64lib STATIC
  src/c64.cpp
  include/c64.h
  src/music_player.cpp
  include/music_player.h
  include/key_scan.h
  include/cia.h
  include/pla.h
  src/irq.S)
target_include_directories(c64lib PUBLIC include)
set_property(TARGET c64lib PROPERTY CXX_STANDARD 17)
set_property(TARGET c64lib PROPERTY PUBLIC)
