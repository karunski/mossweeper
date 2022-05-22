#pragma once

#ifndef MINESWEEPER_TARGET_NES_H
#define MINESWEEPER_TARGET_NES_H

#include "tile_model.h"
#include "input_model.h"

#include <cstdint>
#include <nes.h>
#include <ppu.h>

namespace nes {

  void color_cycle();

  // global declaration of OAM
  inline OAM oam;

  static_assert(alignof(OAM) == 256);

  struct target {
    struct graphics {
      
      static constexpr Color WHITE{Color::Pale, Color::Gray};

      enum class tile_type : std::uint8_t {
      };

      static constexpr auto BLANK = static_cast<tile_type>(0x1);
      static constexpr auto LetterA = static_cast<tile_type>(0x40);
      static constexpr auto SelectArrow = static_cast<tile_type>(0x29);
      static constexpr auto TopLeft = static_cast<tile_type>(0xA);
      static constexpr auto TopBorder = static_cast<tile_type>(0xB);
      static constexpr auto BottomLeft = static_cast<tile_type>(0xF);
      static constexpr auto BottomBorder = static_cast<tile_type>(0x10);
      static constexpr auto LeftBorder = static_cast<tile_type>(0xE);
      static constexpr auto RightBorder = static_cast<tile_type>(0xD);
      static constexpr auto TopRight = static_cast<tile_type>(0xC);
      static constexpr auto BottomRight = static_cast<tile_type>(0x17);
      static constexpr auto HiddenSquare = static_cast<tile_type>(0x0);
      static constexpr tile_type Flag{0x27};

      static constexpr std::uint8_t ScreenWidth = 32;
      static constexpr std::uint8_t ScreenHeight = 30;
      static constexpr std::uint8_t ScoreRows = 2;

      using Emoji = MetaTile<tile_type, 2, 2>;

      static constexpr Emoji Happy = {
          {{tile_type{2}, tile_type{3}}, {tile_type{0x11}, tile_type{0x12}}}};
      static constexpr Emoji Caution = {
          {{tile_type{0x04}, tile_type{0x05}},
           {tile_type{0x13}, tile_type{0x14}}}};

      using Digit = MetaTile<tile_type, 1, 2>;

      static constexpr Digit ScoreDigits[10] = {
          {{tile_type{0x71}, tile_type{0x80}}}, {{tile_type{0x69}, tile_type{0x78}}},
          {{tile_type{0x6a}, tile_type{0x79}}}, {{tile_type{0x6b}, tile_type{0x7a}}},
          {{tile_type{0x6c}, tile_type{0x7b}}}, {{tile_type{0x6d}, tile_type{0x7c}}},
          {{tile_type{0x6e}, tile_type{0x7d}}}, {{tile_type{0x6f}, tile_type{0x7e}}},
          {{tile_type{0x70}, tile_type{0x7f}}}, {{tile_type{0x72}, tile_type{0x81}}}};

      struct Palette {
        Color colors[3];
      };

      static_assert(sizeof(Palette) == 3);
      struct Palettes {
        Color background_color; // shared by all pallettes.
        Palette background[4];
        Palette sprite[4];
      };

      static_assert(sizeof(Palettes) == 25);

      static constexpr Palettes DifficultyScreenPallettes = {
        WHITE, {}
      };

      static constexpr Palettes GameBoardPallettes = {
          WHITE,
          {Color{Color::Light, Color::Yellow},
           Color{Color::Medium, Color::Gray}, Color{}},
          {Color{Color::Dark, Color::Violet},
           Color{Color::Medium, Color::Violet},
           Color{Color::Pale, Color::Violet}}};

      static void
      load_pallettes(const Palettes &pallettes) {
        set_background_color<0>(pallettes.background_color);
        for (uint8_t i = 0; i < 4; i += 1) {
          PPU::copy(PPU::PALETTE_BACKGROUND[i], &pallettes.background[i].colors[0],
                    sizeof(Palette));
        }
        for (uint8_t i = 0; i < 4; i += 1) {
          PPU::copy(PPU::PALETTE_SPRITE[i], &pallettes.sprite[i].colors[0],
                    sizeof(Palette));
        }
      }

      static void render_off() {
        ppu.set_render_control(PPU::render_off);
      }

      static void render_on() {
        ppu.set_render_control(PPU::enable_bg|PPU::enable_sprite);
      }

      template <size_t BgColor> static void set_background_color(Color c) {
        static_assert(BgColor == 0, "Only one background color selection.");
        *PPU::PALETTE_BASE = c;
      }

      static void place(tile_type tile, std::uint8_t x, std::uint8_t y) {
        place(tile, TilePoint{x, y});
      }

      static void place(tile_type tile, const TilePoint & location) {
        if (tile_updates_size == TILE_UPDATES_MAX) {
          return;
        }

        auto & current_update = tile_updates[tile_updates_size];
        current_update.tile = tile;
        current_update.nametable_offset = location.Y*32 + location.X;
        tile_updates_size += 1;
      }

      static void place_immediate(tile_type tile, std::uint8_t x, std::uint8_t y) {
        place_immediate(tile, ppu_coord_addr(x, y));
      }

      static void place_immediate(tile_type tile, PPU::pointer ptr) {
        *ptr = tile;
      }

      static void place(const tile_type *string, std::uint8_t len,
                        std::uint8_t x, std::uint8_t y) {
        if (tile_string_updates_size == TILE_STRING_UPDATES_MAX) {
          return;
        }

        auto & current_update = tile_string_updates[tile_string_updates_size];
        current_update.data = string;
        current_update.nametable_offset = y * 32 + x;
        current_update.len = len;
        tile_string_updates_size += 1;
      }

      static void place_immediate(const tile_type *string, std::uint8_t len,
                        std::uint8_t x, std::uint8_t y) {
        PPU::copy(ppu_coord_addr(x, y), string, len);
        //TODO: set attributes.
      }

      static void fill_immediate(const tile_type tile, std::uint8_t x, std::uint8_t y, std::uint8_t len) {
        PPU::fill(ppu_coord_addr(x, y), tile, len);
      }

      struct sprite_pattern {
        tile_type tile;
        palette_index palette;
        std::uint8_t number_of_frames;
      };

      static constexpr sprite_pattern Cursor{tile_type{0x2a}, PALETTE_0, 7};

      struct sprite {
      public:
        // offsets of the 'sprite' origin from the 'background origin'
        static constexpr std::uint8_t sprite_x_offset = 0;
        static constexpr std::int8_t sprite_y_offset = -2;

        void enable(bool) {
          // sprites are always enabled.  Hide them by moving them off-screen
        }

        void multicolor_enable(bool) {
          // n/a
        }

        void position(std::uint16_t x, std::uint8_t y) {
          oam.set_sprite_position(active_number, x, y);
        }

        void expand(bool, bool) {
          // no effect.
        }

        void activate(std::uint8_t sprite_number,
                      const sprite_pattern &pattern,
                      bool behind_background) {
          active_number = sprite_number;
          oam.set_sprite_tile_index(sprite_number, static_cast<std::uint8_t>(pattern.tile));
          oam.set_sprite_attributes(sprite_number, pattern.palette,
                                    behind_background ? OAM::behind_background
                                                      : OAM::no_attributes);
        }

        void select_frame(const sprite_pattern &pattern, std::uint8_t frame) {
          oam.set_sprite_tile_index(active_number, static_cast<std::uint8_t>(pattern.tile)+frame);
        }

      protected:
        OAM::slot_t active_number = 0;
      };

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

        oam_dma.load_oam(oam);

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

      static constexpr std::uint8_t TILE_STRING_UPDATES_MAX = 5;
      static TileStringUpdate tile_string_updates[TILE_STRING_UPDATES_MAX];
      static std::uint8_t tile_string_updates_size;

      struct TileUpdate {
        tile_type tile;
        std::uint16_t nametable_offset;
      };

      static constexpr std::uint8_t TILE_UPDATES_MAX = 16;
      static TileUpdate tile_updates[TILE_UPDATES_MAX];
      static std::uint8_t tile_updates_size;

      constexpr static PPU::pointer ppu_coord_addr(std::uint8_t x, std::uint8_t y) {
        return PPU::NAME_TABLE_0 + (y * 32 + x);
      }
    };

    static bool startup_check() { return true; }
    
    static void clear_screen() {
      PPU::fill(PPU::NAME_TABLE_0, graphics::BLANK, 960);
      PPU::fill(PPU::ATTRIBUTE_TABLE_0, std::byte{0}, 64);
      ppu.set_scroll(0, 0);
    }

    static std::uint8_t frames_per_second() { return 60; } // TODO... support PAL

    static void load_all_graphics() {
      // not needed with a direct-mapped char rom.
    }

    static auto get_vsync_wait() {
      return nes::get_vsync_wait();
    }

    static unsigned seed_rng() {
      return 0xaaaa;
    }

    static key_scan_res check_keys() {
      nes::controller_1.start_sample();
      key_scan_res result;
      nes::controller_1.stop_sample();
      result.space = nes::controller_1.read_d0_bit();
      /* b button = */ nes::controller_1.read_d0_bit();
      /* select = */ nes::controller_1.read_d0_bit();
      /* start = */ nes::controller_1.read_d0_bit();
      result.w = nes::controller_1.read_d0_bit();
      result.s = nes::controller_1.read_d0_bit();
      result.a = nes::controller_1.read_d0_bit();
      result.d = nes::controller_1.read_d0_bit();
      return result;
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
  inline target::graphics::TileStringUpdate target::graphics::
      tile_string_updates[target::graphics::TILE_STRING_UPDATES_MAX];

  inline std::uint8_t target::graphics::tile_updates_size = 0;
  inline target::graphics::TileUpdate
      target::graphics::tile_updates[target::graphics::TILE_UPDATES_MAX];
}

using target = nes::target;

#endif
