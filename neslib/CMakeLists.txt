add_library(neslib STATIC
  include/nes.h
)
target_include_directories(neslib PUBLIC include)
set_target_properties(neslib PROPERTIES 
  CXX_STANDARD 17
  LINKER_LANGUAGE CXX
)
set_property(TARGET neslib PROPERTY PUBLIC)
