#pragma once

#ifndef MINESWEEPER_TARGET_NES_H
#define MINESWEEPER_TARGET_NES_H

#include <cstdint>
#include <nes.h>
#include <ppu.h>

namespace nes {

  void color_cycle();

  struct target {
    struct graphics {
      static constexpr Color WHITE{Color::Pale, Color::Gray};

      template <size_t BgColor> static void set_background_color(Color c) {
        static_assert(BgColor == 0, "Only one background color selection.");
        ppu.set_render_control(PPU::enable_bg);
        *PPU::PALETTE_BASE = c;
      }
    };

    static bool startup_check() { return true; }
    
    static void clear_screen() {
      // TODO
    }

    static std::uint8_t frames_per_second() { return 60; } // TODO... support PAL

    static void load_tile_set() {
    }

    static auto get_vsync_wait() {
      return nes::get_vsync_wait();
    }

    static unsigned seed_rng() {
      return 0xaaaa;
    }
  };

  void color_cycle() {

    const auto vsyncwait = target::get_vsync_wait();

    // Enable BG rendering.
    vsyncwait();

    ppu.set_render_control(PPU::enable_bg);

    char color = 0;
    for (;;) {
      // Wait for 0.5 second.
      for (int i = 0; i < 30; ++i)
        vsyncwait();

      // Increment the palette color 0.
      *PPU::PALETTE_BASE = ++color;
    }
  }
}

using target = nes::target;

#endif
