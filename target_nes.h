#pragma once

#ifndef MINESWEEPER_TARGET_NES_H
#define MINESWEEPER_TARGET_NES_H

#include <cstdint>
#include <nes.h>
#include <ppu.h>
#include "tile_model.h"

namespace nes {

  void color_cycle();

  struct target {
    struct graphics {
      
      static constexpr Color WHITE{Color::Pale, Color::Gray};

      enum class tile_type : std::uint8_t {
      };

      static constexpr auto BLANK = static_cast<tile_type>(0x1);
      static constexpr auto LetterA = static_cast<tile_type>(0x40);
      static constexpr auto SelectArrow = static_cast<tile_type>(0xF);

      static constexpr std::uint8_t ScreenWidth = 32;
      static constexpr std::uint8_t ScreenHeight = 30;
      
      static void render_off() {
        ppu.set_render_control(PPU::render_off);
      }

      static void enable_render_background() {
        ppu.set_render_control(PPU::enable_bg);
      }

      template <size_t BgColor> static void set_background_color(Color c) {
        static_assert(BgColor == 0, "Only one background color selection.");
        *PPU::PALETTE_BASE = c;
      }

      static void place(tile_type tile, const TilePoint & location) {
        auto & current_update = tile_updates[tile_updates_size];
        current_update.tile = tile;
        current_update.nametable_offset = location.Y*32 + location.X;
        tile_updates_size += 1;
      }

      static void place(const tile_type *string, std::uint8_t len,
                        std::uint8_t x, std::uint8_t y) {
        auto & current_update = tile_string_updates[tile_string_updates_size];
        current_update.data = string;
        current_update.nametable_offset = y * 32 + x;
        current_update.len = len;
        tile_string_updates_size += 1;
      }

      static void finish_rendering() {

        // Multi-byte updates.
        for (std::uint8_t i = 0; i < tile_string_updates_size; i += 1) {
          const auto & current_update = tile_string_updates[i];
          PPU::copy(PPU::NAME_TABLE_0 + current_update.nametable_offset, current_update.data, current_update.len);
        }

        tile_string_updates_size = 0;

        // Single byte updates.
        for (std::uint8_t i = 0; i < tile_updates_size; i += 1) {
          const auto & current_update = tile_updates[i];
          *(PPU::NAME_TABLE_0 + current_update.nametable_offset) = current_update.tile;
        }

        tile_updates_size = 0;
        
        // must always reset before the frame starts... the on-screen rendering uses the address
        // register to determine where to read the tiles from!
        ppu.set_ppu_address(PPU::NAME_TABLE_0);
      }

    private:

      struct TileStringUpdate {
        const tile_type *data;
        std::uint16_t nametable_offset;
        std::uint8_t len;
      };

      static TileStringUpdate tile_string_updates[5];
      static std::uint8_t tile_string_updates_size;

      struct TileUpdate {
        tile_type tile;
        std::uint16_t nametable_offset;
      };

      static TileUpdate tile_updates[10];
      static std::uint8_t tile_updates_size;
    };

    static bool startup_check() { return true; }
    
    static void clear_screen() {
      PPU::fill(PPU::NAME_TABLE_0, graphics::BLANK, 960);
      ppu.set_scroll(0, 0);
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

  inline std::uint8_t target::graphics::tile_string_updates_size = 0;
  inline target::graphics::TileStringUpdate target::graphics::tile_string_updates[5];

  inline std::uint8_t target::graphics::tile_updates_size = 0;
  inline target::graphics::TileUpdate target::graphics::tile_updates[10];
}

using target = nes::target;

#endif
