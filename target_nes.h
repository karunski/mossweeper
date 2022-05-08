#pragma once

#ifndef MINESWEEPER_TARGET_NES_H
#define MINESWEEPER_TARGET_NES_H

#include <cstdint>

namespace nes {
  struct target {

    static bool startup_check() { return true; }
    
    static void clear_screen() {
      // TODO
    }

    static std::uint8_t frames_per_second() { return 60; } // TODO... support PAL

    static void load_tile_set() {
      // TODO
    }
  };
}

using target = nes::target;

#endif
