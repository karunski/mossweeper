#pragma once

#ifndef MINESWEEPER_TARGET_C64_H
#define MINESWEEPER_TARGET_C64_H

#include "tile_model.h"
#include "input_model.h"

#include <c64.h>
#include <stdio.h>
#include <key_scan.h>
#include <sid.h>
#include <music_player.h>
#include <pla.h>
#include <cstdint>

extern "C" {
extern c64::Sprite sprite_data_ram[32];
extern c64::Sprite sprite_data_end;
extern const std::byte minesweeper_gfx[2048];
extern const c64::Sprite minesweeper_cursor[7];
extern const c64::Sprite minesweeper_bg_sprites[1];
extern const c64::ColorCode minesweeper_color[256];
}

namespace c64 {

// Use this hardware configuration for the RAM associated with the display:
using ScreenMemoryAddresses =
    c64::DisplayAddr<c64::CIA2::vic_bank::NO_3, c64::VicII::VIDEO_OFFSET_0000,
                     c64::VicII::TEXT_0800>;

CharRAM &char_data_ram = ScreenMemoryAddresses::chars;
ScreenRAM &screen_ram = ScreenMemoryAddresses::screen;

struct target {

  struct graphics {

    using color_type = ColorCode;
    using tile_type = ScreenCode;
    using chr_code_type = ScreenCode;
    using TileType = tile_type;

    using C64Emoji = MetaTile<ScreenCode, 2, 2>;
    using C64Digit = MetaTile<ScreenCode, 1, 2>;

    // Tile Identifier for a blank / white tile.
    static constexpr auto BLANK = ScreenCode{1};
    static constexpr auto TopLeft = ScreenCode{8};
    static constexpr auto TopRight = ScreenCode{5};
    static constexpr auto BottomLeft = ScreenCode{39};
    static constexpr auto BottomRight = ScreenCode{36};
    static constexpr auto TopBorder = ScreenCode{4};
    static constexpr auto RightBorder = ScreenCode{37};
    static constexpr auto BottomBorder = ScreenCode{6};
    static constexpr auto LeftBorder = ScreenCode{7};
    static constexpr auto HiddenSquare = ScreenCode{0};
    static constexpr auto Mine = TileType{25};
    static constexpr auto Flag = TileType{26};
    static constexpr auto Wrong = TileType{27};
    // assume ExposedSquare doubles as '0' marker; and '1', '2', ... '8'
    // are in the tiles following it.
    static constexpr auto ExposedSquare = TileType{16};
    // Alphabet chars must follow alphabetically after LetterA
    static constexpr auto LetterA = TileType{64};
    static constexpr auto SelectArrow = TileType{32};

    static constexpr auto ScoreRows = std::uint8_t{2};
    static constexpr auto ScoreSize = std::uint8_t{3};
    static constexpr auto CanExpandSprites = false;

    static constexpr auto NumberMarker(std::uint8_t idx) {
      return static_cast<TileType>(static_cast<std::uint8_t>(ExposedSquare) +
                                   idx);
    }

    static constexpr C64Digit ScoreDigits[10] = {
        {{TileType{15}, TileType{46}}}, {{TileType{9}, TileType{41}}},
        {{TileType{10}, TileType{42}}}, {{TileType{10}, TileType{43}}},
        {{TileType{11}, TileType{45}}}, {{TileType{12}, TileType{43}}},
        {{TileType{12}, TileType{44}}}, {{TileType{13}, TileType{41}}},
        {{TileType{14}, TileType{44}}}, {{TileType{14}, TileType{43}}}};

    static constexpr C64Emoji Happy = {
        {{TileType{2}, TileType{3}}, {TileType{34}, TileType{35}}}};

    static constexpr C64Emoji Caution = {
        {{TileType{49}, TileType{50}}, {TileType{51}, TileType{52}}}};

    static constexpr C64Emoji Dead = {
        {{TileType{53}, TileType{54}}, {TileType{55}, TileType{56}}}};

    static constexpr C64Emoji Win = {
        {{TileType{57}, TileType{58}}, {TileType{34}, TileType{35}}}};

    static_assert(Happy.tiles[0][1] == c64::ScreenCode{3});
    static_assert(Happy.tiles[1][0] == c64::ScreenCode{34});

    static constexpr auto WHITE = color_type::WHITE;

    static constexpr std::uint8_t ScreenWidth = 40;
    static constexpr std::uint8_t ScreenHeight = 25;

    template<size_t BgColorIndex>
    static void set_background_color(color_type color) {
      c64::vic_ii.background_color[BgColorIndex] = color;
    }

    static void render_off() { }// TODO / not needed 

    static void render_on() {
      // TODO / not needed
    }

    static void finish_rendering() {}

    static void place(ScreenCode Tile, std::uint8_t x, std::uint8_t y) {
      c64::screen_ram.at(x, y) = Tile;
      c64::color_ram.at(x, y) = minesweeper_color[static_cast<uint8_t>(Tile)];
    }

    static void place_immediate(ScreenCode Tile, std::uint8_t x, std::uint8_t y) {
      place(Tile, x, y);
    }

    struct tile_to_char {
      static constexpr auto call(char c) {
        return c == ' ' ? BLANK
                        : static_cast<TileType>(
                              static_cast<std::uint8_t>(LetterA) + (c - 'A'));
      };
    };

    static void place_immediate(const ScreenCode *string, std::uint8_t len,
                                std::uint8_t x, std::uint8_t y) {
      for (std::uint8_t i = 0; i < len; i += 1) {
        place_immediate(string[i], x + i, y);
      }
    }

    static void fill_immediate(ScreenCode tile, std::uint8_t x, std::uint8_t y,
                               std::uint8_t len) {
      for (std::uint8_t i = 0; i < len; i += 1) {
        place_immediate(tile, x + i, y);
      }
    }

    static void place(ScreenCode Tile, TilePoint tilePos) {
      place(Tile, tilePos.X, tilePos.Y);
    }

    struct Pallettes {
      color_type background_color;
    };
    static constexpr Pallettes GameBoardPallettes{WHITE};
    static constexpr Pallettes DifficultyScreenPallettes{WHITE};

    static void load_pallettes(const Pallettes & pallettes) {
      c64::vic_ii.set_multi_color_mode(false);
      set_background_color<0>(pallettes.background_color);
    }

    struct sprite_pattern {
      std::uint8_t slot; // location within the sprite memory area
      std::uint8_t number_of_frames;
      ColorCode sprite_color;
    };

    static const sprite_pattern Cursor;
    static const sprite_pattern SpriteBackground;

    struct sprite {
    public:
      static constexpr std::uint8_t sprite_x_offset = 24;
      static constexpr std::uint8_t sprite_y_offset = 50;

      struct Position {
        std::uint16_t X;
        std::uint8_t Y;
      };

      void enable(bool en) { c64::vic_ii.sprite_enable.set(active_number, en); }

      void multicolor_enable(bool en) {
        c64::vic_ii.sprite_multicolor_enable.set(active_number, en);
      }

      void position(std::uint16_t x, std::uint8_t y) {
        c64::vic_ii.set_sprite_pos(active_number, x, y);
      }

      void expand(bool x, bool y) {
        c64::vic_ii.sprite_x_expansion.set(active_number, x);
        c64::vic_ii.sprite_y_expansion.set(active_number, y);
      }

      void activate(std::uint8_t sprite_number, const sprite_pattern & pattern, bool behind_background) {
        active_number = sprite_number;
        c64::screen_ram.sprite_ptr(active_number) = sprite_data_ram[pattern.slot];
        c64::vic_ii.sprite_color[active_number] = pattern.sprite_color;
        c64::vic_ii.sprite_data_priority.set(active_number, behind_background);
      }

      void select_frame(const sprite_pattern &pattern, std::uint8_t frame) {
        c64::screen_ram.sprite_ptr(active_number) =
            sprite_data_ram[pattern.slot + frame];
      }

    protected:
      std::uint8_t active_number = 0;
    };
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
      screen_mem[i] = graphics::BLANK;
    }

    for (std::uint8_t i = 0; i < 8; i += 1) {
      c64::vic_ii.set_sprite_pos(i, 0, 0);
    }
  }

  static std::uint8_t frames_per_second() {
    return count_raster() > 263 ? 50 : 60;
  }

  static void load_tile_set() {
    c64::cia2.set_vic_bank(c64::ScreenMemoryAddresses::vic_base_setting);
    memcpy(c64::char_data_ram.data, minesweeper_gfx, sizeof(minesweeper_gfx));
  }

  static void load_sprite_data() {
    // Temporarily bank-out the kernel and basic ROMS, and I/O, to use the RAM they
    // shadow to hold sprite data.
    const ScopedInterruptDisable disable_interrupts;
    const PLA::BankSwitchScope disable_io{PLA::MODE_28};

    // Put the cursor sprite in VRAM.
    for (std::uint8_t i = 0; i < target::graphics::Cursor.number_of_frames;
         i += 1) {
      sprite_data_ram[target::graphics::Cursor.slot + i] =
          minesweeper_cursor[i];
    }

    sprite_data_ram[target::graphics::SpriteBackground.slot] =
        minesweeper_bg_sprites[0];
  }

  static void load_all_graphics() {
    load_tile_set();
    load_sprite_data();
    // Our graphics data is in place, now configure the hardware to point to it.
    c64::vic_ii.setup_memory(ScreenMemoryAddresses::screen_setting,
                             ScreenMemoryAddresses::char_data_setting);
  }

  static auto get_vsync_wait() { return c64::get_vsync_wait(); }

  static unsigned seed_rng() {
    sid.voices[2].set_frequency(0xFFFF);
    sid.voices[2].set_control(SIDVoice::noise);
    return (get_non_zero_sid_random() << 8) | get_non_zero_sid_random();
  }

  static key_scan_res check_keys() {
    c64::JoyScanner joyscan;

    if (const auto joystick_state = joyscan.scan_joysticks();
        joystick_state.joystick_a != c64::JoyScanner::JoyScan::CLEAR ||
        joystick_state.joystick_b != c64::JoyScanner::JoyScan::CLEAR) {
      return key_scan_res{
          c64::JoyScanner::JoyScan::BUTTON & joystick_state.joystick_a,
          false, // secondary fire
          c64::JoyScanner::JoyScan::UP & joystick_state.joystick_a,
          c64::JoyScanner::JoyScan::LEFT & joystick_state.joystick_a,
          c64::JoyScanner::JoyScan::DOWN & joystick_state.joystick_a,
          c64::JoyScanner::JoyScan::RIGHT & joystick_state.joystick_a};
    }

    c64::KeyScanner scanner;
    const auto scanRow0 = scanner.Row<0>();
    const auto scanRow1 = scanner.Row<1>();
    const auto scanRow2 = scanner.Row<2>();
    const auto scanRow6 = scanner.Row<6>();
    const auto scanRow7 = scanner.Row<7>();
    const auto down = scanRow0.KEY_CURSOR_DOWN();
    const auto right = scanRow0.KEY_CURSOR_RIGHT();
    const auto shift = scanRow1.KEY_LEFT_SHIFT() || scanRow6.KEY_RIGHT_SHIFT();
    return key_scan_res{scanRow7.KEY_SPACE() || scanRow0.KEY_RETURN(),
                        false,
                        scanRow1.KEY_W() || (down && shift),
                        scanRow1.KEY_A() || (right && shift),
                        scanRow1.KEY_S() || (down && !shift),
                        scanRow2.KEY_D() || (right && !shift)};
  }

private:
  
  static uint8_t get_non_zero_sid_random() {
    for (;;) {
      const auto randbits = sid.oscillator_3;
      if (randbits) {
        return randbits;
      }
    }
  }

  static std::uint16_t count_raster() {
    static std::uint16_t count_raster;
    for (count_raster = 0;;) {
      const auto poll_raster = vic_ii.get_raster();
      if (poll_raster >= count_raster) {
        count_raster = poll_raster;
      } else {
        return count_raster;
      }
    }
  }
};

inline const target::graphics::sprite_pattern target::graphics::Cursor{
    0, 7, minesweeper_cursor[0].mode.sprite_color()};

inline const target::graphics::sprite_pattern
    target::graphics::SpriteBackground{
        7, 1, minesweeper_bg_sprites[0].mode.sprite_color()};
}

using target = c64::target;

#endif
