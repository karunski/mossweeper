#pragma once

#ifndef MINESWEEPER_TARGET_C64_H
#define MINESWEEPER_TARGET_C64_H

#include <c64.h>
#include <stdio.h>
#include <key_scan.h>
#include <sid.h>
#include <music_player.h>
#include <pla.h>

extern "C" {
extern c64::Sprite sprite_data_ram[32];
extern c64::Sprite sprite_data_end;
extern const std::byte minesweeper_gfx[2048];
extern const c64::Sprite minesweeper_cursor[7];
extern const c64::Sprite minesweeper_bg_sprites[1];
extern const c64::ColorCode minesweeper_color[256];
}

namespace c64 {

using ScreenMemoryAddresses =
    c64::DisplayAddr<c64::CIA2::vic_bank::NO_3, c64::VicII::VIDEO_OFFSET_0000,
                     c64::VicII::TEXT_0800>;

CharRAM &char_data_ram = ScreenMemoryAddresses::chars;
ScreenRAM &screen_ram = ScreenMemoryAddresses::screen;

struct target {

  struct graphics_traits {
    // Tile Identifier for a blank / white tile.
    static constexpr auto BLANK = ScreenCode{1};
  };

  static bool startup_check() {
    if (&char_data_ram != reinterpret_cast<void *>(0xC800)) {
      puts("WRONG CHAR MEMORY OFFSET");
      return false;
    }

    return true;
  }

  static void clear_screen() {
    auto *screen_mem = screen_ram.data();

    for (unsigned i = 0; i < 1000; i += 1) {
      screen_mem[i] = graphics_traits::BLANK;
    }

    for (std::uint8_t i = 0; i < 8; i += 1) {
      c64::vic_ii.set_sprite_pos(i, 0, 0);
    }
  }
};

}

using target = c64::target;

#endif