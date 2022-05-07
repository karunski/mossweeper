#pragma once

#ifndef MINESWEEPER_TARGET_NES_H
#define MINESWEEPER_TARGET_NES_H

namespace nes {
  struct target {

    static bool startup_check() { return true; }
    
    static void clear_screen() {
      // TODO
    }
  };
}

using target = nes::target;

#endif
