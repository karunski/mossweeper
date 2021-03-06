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

  namespace target {
  struct graphics {

    static constexpr Color WHITE{Color::Pale, Color::Gray};

    enum class tile_type : std::uint8_t {};

    using chr_code_type = std::uint8_t;

    static constexpr chr_code_type chr_code_atlas[] = {
        0x1,  // Blank,       0
        0x29, // SelectArrow  1
        0x0,  // Hidden       2
        0x26, // mine         3
        // Letter A-H         4-13
        0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
        // Letter J-T         14-23
        0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53,
        // Letter U-Z         24-29
        0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
        0x0a, // TopLeft       30
        0x0b, // TopBorder     31
        0x0c, // TopRight      32
        0x0f, // BottomLeft    33
        0x10, // BottomBorder  34
        0x0e, // LeftBorder    35
        0x0d, // RightBorder   36
        0x17, // BottomRight   37
        0x28, // Wrong         38
        0x27, // Flag          39
        // Happy 40-43
        0x02, 0x03, 0x11, 0x12,
        // Caution 44-47
        0x04, 0x05, 0x13, 0x14,
        // Dead 48-51
        0x06, 0x07, 0x15, 0x16,
        // Win  52-55
        0x08, 0x09, 0x11, 0x12,
        // Digit 0 56-57
        0x71, 0x80,
        // Digit 1 58-59
        0x69, 0x78,
        // Digit 2 60-61
        0x6a, 0x79,
        // Digit 3 62-63
        0x6a, 0x81,
        // Digit 4 64-65
        0x6c, 0x7b,
        // Digit 5 66-67
        0x6d, 0x81,
        // Digit 6 68-69
        0x6d, 0x7d,
        // Digit 7 70-71
        0x6f, 0x78,
        // Digit 8 72-73
        0x72, 0x7d,
        // Digit 9 74-75
        0x72, 0x81,
        // NumberMarker 76-84
        0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25,
        0x1, // Blank w/ border attr 85
        0x00, 0x00};

    static constexpr chr_code_type tile_to_chr_code(const tile_type t) {
      return chr_code_atlas[static_cast<std::uint8_t>(t)];
    }

    struct palette_index_quartet {
      std::uint8_t pal_0 : 2;
      std::uint8_t pal_1 : 2;
      std::uint8_t pal_2 : 2;
      std::uint8_t pal_3 : 2;
    };

    static constexpr palette_index_quartet palette_atlas[] = {
        {0,  // BLANK
         0,  // selectArrow
         2,  // Hidden,
         0}, // mine
        // Letter A-D
        {0, 0, 0, 0},
        // Letter E-H
        {0, 0, 0, 0},
        // Letter I-L
        {0, 0, 0, 0},
        // Letter M-P
        {0, 0, 0, 0},
        // Letter Q-T
        {0, 0, 0, 0},
        // Letter U-X
        {0, 0, 0, 0},
        {0,  // letter Y
         0,  // Letter Z
         1,  // TopLeft
         1}, // TopBorder
        {
            1, // TopRight 32
            1, // BottomLeft
            1, // BottomBorder 34
            1  // LeftBorder
        },
        {
            1, // RightBorder 36
            1, // BottomRight 37
            2, // Wrong       38
            2, // Flag        39
        },
        // Happy 40-43
        {0, 0, 0, 0},
        // Caution 44-47
        {0, 0, 0, 0},
        // Dead 48-51
        {0, 0, 0, 0},
        // Win 52-55
        {0, 0, 0, 0},
        {// Digit 0 56-57
         3, 3,
         // Digit 1 58-59
         3, 3},
        {// Digit 2 60-61
         3, 3,
         // Digit 3 62-63
         3, 3},
        {// Digit 4 64-65
         3, 3,
         // Digit 5 66-67
         3, 3},
        {// Digit 6 68-69
         3, 3,
         // digit 7 70-71
         3, 3},
        {// Digit 8 72-73
         3, 3,
         // Digit 9 74-75
         3, 3},
        {2, 2, 2, 2}, // Number marker 0-3 76-79,
        {2, 2, 2, 2}, // Number marker 4-7 80-83,
        {
            2, // number marker 8 84
            1, // BLANK w/ border attribute 85
        }

    };

    static constexpr std::uint8_t tile_to_palette_idx(const tile_type t) {
      const auto &quartet = palette_atlas[static_cast<std::uint8_t>(t) >> 2];
      switch (static_cast<std::uint8_t>(t) & 0b11) {
      case 0:
        return quartet.pal_0;
      case 1:
        return quartet.pal_1;
      case 2:
        return quartet.pal_2;
      case 3:
      default:
        return quartet.pal_3;
      }
    }

    static_assert(sizeof(palette_atlas) == sizeof(chr_code_atlas) / 4);

    static constexpr tile_type BLANK{0};
    static constexpr tile_type BLANK_BORDER{85};
    static constexpr tile_type LetterA{0x04};
    static constexpr tile_type SelectArrow{0x01};
    static constexpr tile_type HiddenSquare{2};
    static constexpr tile_type Mine{3};
    static constexpr MetaTile<tile_type, 2, 2> TopLeft{
        {{BLANK_BORDER, BLANK_BORDER}, {BLANK_BORDER, tile_type{30}}}};
    static constexpr MetaTile<tile_type, 1, 2> TopBorder{
        {{BLANK_BORDER}, {tile_type{31}}}};
    static constexpr MetaTile<tile_type, 1, 2> TopRight{
        {{BLANK_BORDER}, {tile_type{32}}}};
    static constexpr MetaTile<tile_type, 2, 1> BottomLeft{
        {{BLANK_BORDER, tile_type{33}}}};
    static constexpr tile_type BottomBorder{34};
    static constexpr MetaTile<tile_type, 2, 1> LeftBorder{
        {{BLANK_BORDER, tile_type{35}}}};
    static constexpr tile_type RightBorder{36};
    static constexpr tile_type BottomRight{37};
    static constexpr tile_type Wrong{38};
    static constexpr tile_type Flag{39};

    static constexpr std::uint8_t ScreenWidth = 34;
    static constexpr std::uint8_t WindowWidth = 32;
    static constexpr std::uint8_t ScreenHeight = 30;
    static constexpr std::uint8_t ScoreRows = 2;
    static constexpr std::uint8_t ScoreSize = 4;
    static constexpr bool CanExpandSprites = false;
    static constexpr bool GameBoardHeightMustBeEven = true;

    using Emoji = MetaTile<tile_type, 2, 2>;

    static constexpr Emoji Happy = {
        {{tile_type{40}, tile_type{41}}, {tile_type{42}, tile_type{43}}}};
    static constexpr Emoji Caution = {
        {{tile_type{44}, tile_type{45}}, {tile_type{46}, tile_type{47}}}};
    static constexpr Emoji Dead = {
        {{tile_type{48}, tile_type{49}}, {tile_type{50}, tile_type{51}}}};
    static constexpr Emoji Win = {
        {{tile_type{52}, tile_type{53}}, {tile_type{54}, tile_type{55}}}};

    using Digit = MetaTile<tile_type, 1, 2>;

    static constexpr Digit ScoreDigits[10] = {
        {{tile_type{56}, tile_type{57}}}, {{tile_type{58}, tile_type{59}}},
        {{tile_type{60}, tile_type{61}}}, {{tile_type{62}, tile_type{63}}},
        {{tile_type{64}, tile_type{65}}}, {{tile_type{66}, tile_type{67}}},
        {{tile_type{68}, tile_type{69}}}, {{tile_type{70}, tile_type{71}}},
        {{tile_type{72}, tile_type{73}}}, {{tile_type{74}, tile_type{75}}}};

    static constexpr auto NumberMarker(std::uint8_t idx) {
      return static_cast<tile_type>(76 + idx);
    }

    struct Palette {
      Color colors[3];
    };

    static_assert(sizeof(Palette) == 3);
    struct Palettes {
      Color background_color; // shared by all palettes.
      Palette background[4];
      Palette sprite[4];
    };

    static_assert(sizeof(Palettes) == 25);

    static constexpr Palettes DifficultyScreenPalettes = {
        WHITE,
        {{Color{Color::Dark, Color::Black}, Color{Color::Dark, Color::Black},
          Color{Color::Dark, Color::Black}},
         {Color{Color::Dark, Color::Black}, Color{Color::Dark, Color::Black},
          Color{Color::Dark, Color::Black}},
         {Color{Color::Dark, Color::Black}, Color{Color::Dark, Color::Black},
          Color{Color::Dark, Color::Black}},
         {Color{Color::Dark, Color::Black}, Color{Color::Dark, Color::Black},
          Color{Color::Dark, Color::Black}}}};

    static constexpr Palettes GameBoardPalettes = {
        {Color::Pale, Color::Black},
        {// Palette 0: smiley face
         {Color{Color::Light, Color::Yellow}, Color{Color::Pale, Color::Black},
          Color{Color::Dark, Color::Black}},
         // Palette 1: borders
         {Color{Color::Pale, Color::Gray}, Color{Color::Dark, Color::Gray},
          Color{Color::Dark, Color::Black}},
         // Palette 2: game squares
         {Color{Color::Medium, Color::Green}, Color{Color::Medium, Color::Blue},
          Color{Color::Dark, Color::Gray}},
         // Palette 3: Digits
         {Color{Color::Light, Color::Red}, Color{Color::Dark, Color::Red},
          Color{Color::Dark, Color::Black}}},

        {Color{Color::Dark, Color::Violet}, Color{Color::Medium, Color::Violet},
         Color{Color::Light, Color::Violet}}};

    static constexpr Palettes ResetButtonPalettes = {
        {Color::Pale, Color::Black},
        {// Palette 0: smiley face
         {Color{Color::Light, Color::Yellow},
          Color{Color::Medium, Color::Violet},
          Color{Color::Dark, Color::Black}},
         // Palette 1: borders
         {Color{Color::Pale, Color::Gray}, Color{Color::Dark, Color::Gray},
          Color{Color::Dark, Color::Black}},
         // Palette 2: game squares
         {Color{Color::Medium, Color::Green}, Color{Color::Medium, Color::Blue},
          Color{Color::Dark, Color::Gray}},
         // Palette 3: Digits
         {Color{Color::Light, Color::Red}, Color{Color::Dark, Color::Red},
          Color{Color::Dark, Color::Black}}},

        {Color{Color::Dark, Color::Violet}, Color{Color::Medium, Color::Violet},
         Color{Color::Pale, Color::Violet}}};

    static void load_palettes(const Palettes &pallettes) {
      set_background_color<0>(pallettes.background_color);
      for (uint8_t i = 0; i < 4; i += 1) {
        PPU::copy(PPU::PALETTE_BACKGROUND[i],
                  &pallettes.background[i].colors[0], sizeof(Palette));
      }
      for (uint8_t i = 0; i < 4; i += 1) {
        PPU::copy(PPU::PALETTE_SPRITE[i], &pallettes.sprite[i].colors[0],
                  sizeof(Palette));
      }
    }

    static void load_palettes_defer(const Palettes &palettes) {
      next_palettes = &palettes;
    }

    static void render_off() {
      ppu_wait_vblank();
      ppu.set_render_control(PPU::render_off);
    }

    static void render_on() {
      ppu_wait_vblank();
      ppu.set_render_control(PPU::enable_bg | PPU::enable_sprite |
                             PPU::enable_bg_column_0);
    }

    template <size_t BgColor> static void set_background_color(Color c) {
      static_assert(BgColor == 0, "Only one background color selection.");
      *PPU::PALETTE_BASE = c;
    }

    static void place(tile_type tile, std::uint8_t x, std::uint8_t y) {
      place(tile, TilePoint{x, y});
    }

    static constexpr std::uint8_t attr_byte_mask(std::uint8_t attr_x,
                                                 std::uint8_t attr_y) {
      constexpr std::uint8_t mask_bits[2][2] = {{0b00000011, 0b00001100},
                                                {0b00110000, 0b11000000}};
      return mask_bits[attr_y & 0b1][attr_x & 0b1];
    }

    struct attr_point {
      std::uint8_t x;
      std::uint8_t y;

      constexpr std::uint8_t attr_table_offset() const {
        if (x > 15) {
          return 64 + ((y >> 1) * 8) + ((x - 16) >> 1);
        }
        return (y >> 1) * 8 + (x >> 1);
      }

      constexpr std::uint8_t attr_byte_shift() const {
        constexpr std::uint8_t shift_amt[2][2] = {{0, 2}, {4, 6}};
        return shift_amt[y & 0b1][x & 0b1];
      }
    };

    constexpr static bool is_attr_tile(std::uint8_t x, std::uint8_t y) {
      return (x & 0b1) == 0 && (y & 0b1) == 0;
    }

    static constexpr attr_point
    tile_point_to_attr_point(const TilePoint &location) {
      return attr_point{static_cast<std::uint8_t>(location.X >> 1),
                        static_cast<std::uint8_t>(location.Y >> 1)};
    }

    static void place(tile_type tile, const TilePoint &location) {
      if (tile_updates_size == TILE_UPDATES_MAX) {
        return;
      }

      {
        auto &current_update = tile_updates[tile_updates_size];
        current_update.tile = tile_to_chr_code(tile);
        current_update.nametable_offset = location.Y * 32 + location.X;
        tile_updates_size += 1;
      }
    }

    static void update_attr(std::uint8_t attr_offset, std::byte mask,
                            std::byte attr_data) {
      const auto attr_byte_ptr =
          attr_offset > 64 ? PPU::ATTRIBUTE_TABLE_1 + (attr_offset - 64)
                           : PPU::ATTRIBUTE_TABLE_0 + attr_offset;
      std::byte current_val = (*attr_byte_ptr).fetch_byte<std::byte>();
      *attr_byte_ptr = (current_val & ~(mask)) | attr_data;
    }

    static void place_attr_immediate(tile_type tile, std::uint8_t x,
                                     std::uint8_t y) {
      const auto location_attr = tile_point_to_attr_point(TilePoint{x, y});
      const auto shift = location_attr.attr_byte_shift();

      update_attr(location_attr.attr_table_offset(), std::byte{0b11} << shift,
                  std::byte{tile_to_palette_idx(tile)} << shift);
    }

    static void place_immediate(tile_type tile, std::uint8_t x,
                                std::uint8_t y) {
      place_immediate(tile, ppu_coord_addr(x, y));
      if (is_attr_tile(x, y)) {
        place_attr_immediate(tile, x, y);
      }
    }

    static void place_immediate(tile_type tile, PPU::pointer ptr) {
      *ptr = chr_code_atlas[static_cast<std::uint8_t>(tile)];
    }

    static void place(const tile_type *string, std::uint8_t len, std::uint8_t x,
                      std::uint8_t y) {
      if (tile_string_updates_size == TILE_STRING_UPDATES_MAX) {
        return;
      }

      auto &current_update = tile_string_updates[tile_string_updates_size];
      current_update.data = string;
      current_update.nametable_offset = y * 32 + x;
      current_update.len = len;
      tile_string_updates_size += 1;
    }

    struct tile_to_char {
      static constexpr auto call(char c) {
        return chr_code_atlas
            [c == ' ' ? static_cast<std::uint8_t>(BLANK)
                      : (c >= '1' && c <= '8'
                             ? static_cast<std::uint8_t>(NumberMarker(c - '0'))
                             : (c == '0' ? (static_cast<std::uint8_t>(LetterA) +
                                            ('O' - 'A'))
                                         : (static_cast<std::uint8_t>(LetterA) +
                                            (c - 'A'))))];
      };
    };

    static void place_immediate(const std::uint8_t *string, std::uint8_t len,
                                std::uint8_t x, std::uint8_t y) {
      PPU::copy(ppu_coord_addr(x, y), string, len);
    }

    static void fill_immediate(const tile_type tile, std::uint8_t x,
                               std::uint8_t y, std::uint8_t len) {
      PPU::fill(ppu_coord_addr(x, y), graphics::tile_to_chr_code(tile), len);
      if ((y & 0b1) == 0) {
        len -= (x & 0b1);
        x += (x & 0b1);

        for (std::uint8_t i = x; i < (x + len); i += 2) {
          place_attr_immediate(tile, i, y);
        }
      }
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
      static std::int8_t sprite_x_offset;
      static constexpr std::int8_t sprite_y_offset = -3;

      void enable(bool) {
        // sprites are always enabled.  Hide them by moving them off-screen
        oam.set_sprite_position(active_number, 0xFF, 0xFF);
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

      void activate(std::uint8_t sprite_number, const sprite_pattern &pattern,
                    bool behind_background) {
        active_number = sprite_number;
        oam.set_sprite_tile_index(sprite_number,
                                  static_cast<std::uint8_t>(pattern.tile));
        oam.set_sprite_attributes(sprite_number, pattern.palette,
                                  behind_background ? OAM::behind_background
                                                    : OAM::no_attributes);
      }

      void select_frame(const sprite_pattern &pattern, std::uint8_t frame) {
        oam.set_sprite_tile_index(
            active_number, static_cast<std::uint8_t>(pattern.tile) + frame);
      }

    protected:
      OAM::slot_t active_number = 0;
    };

    static void scroll_tile_x(std::uint8_t offset_tile_x) {
      window_base = PPU::NAME_TABLE_0 + offset_tile_x;
      sprite::sprite_x_offset = -(8 * offset_tile_x);
    }

    static void finish_rendering() {

      // Multi-byte updates.
      for (std::uint8_t i = 0; i < tile_string_updates_size; i += 1) {
        const auto &current_update = tile_string_updates[i];
        PPU::copy(PPU::NAME_TABLE_0 + current_update.nametable_offset,
                  current_update.data, current_update.len);
      }

      tile_string_updates_size = 0;

      // Single byte updates.
      for (std::uint8_t i = 0; i < tile_updates_size; i += 1) {
        const auto &current_update = tile_updates[i];
        *(PPU::NAME_TABLE_0 + current_update.nametable_offset) =
            current_update.tile;
      }

      tile_updates_size = 0;

      if (next_palettes) {
        load_palettes(*next_palettes);
        next_palettes = nullptr;
      }

      // must always reset before the frame starts... the on-screen rendering
      // uses the address register to determine where to read the tiles from!
      ppu.set_ppu_address(window_base);

      oam_dma.load_oam(oam);
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
      std::uint8_t tile;
      std::uint16_t nametable_offset;
    };

    static constexpr std::uint8_t TILE_UPDATES_MAX = 20;
    static TileUpdate tile_updates[TILE_UPDATES_MAX];
    static std::uint8_t tile_updates_size;

    static const Palettes *next_palettes;
    static PPU::pointer window_base;

    constexpr static PPU::pointer ppu_coord_addr(std::uint8_t x,
                                                 std::uint8_t y) {
      if (x > 31) {
        x -= 32;
        return PPU::NAME_TABLE_1 + (y * 32 + x);
      }

      return PPU::NAME_TABLE_0 + (y * 32 + x);
    }
  };

  bool startup_check() { return true; }

  void clear_screen() {
    PPU::fill(PPU::NAME_TABLE_0, graphics::tile_to_chr_code(graphics::BLANK),
              960);
    PPU::fill(PPU::ATTRIBUTE_TABLE_0,
              graphics::tile_to_palette_idx(graphics::BLANK), 64);
    PPU::fill(PPU::NAME_TABLE_1, graphics::tile_to_chr_code(graphics::BLANK),
              960);
    PPU::fill(PPU::ATTRIBUTE_TABLE_1,
              graphics::tile_to_palette_idx(graphics::BLANK), 64);
    ppu.set_scroll(0, 0);
  }

  std::uint8_t frames_per_second() { return 60; } // TODO... support PAL

  void load_all_graphics() {
    // not needed with a direct-mapped char rom.
  }

  auto get_vsync_wait() { return nes::get_vsync_wait(); }

  unsigned seed_rng() { return 0xaaaa; }

  key_scan_res check_keys() {
    nes::controller_1.start_sample();
    key_scan_res result;
    nes::controller_1.stop_sample();
    result.space = nes::controller_1.read_d0_bit();
    result.fire_secondary = nes::controller_1.read_d0_bit();
    /* select = */ nes::controller_1.read_d0_bit();
    /* start = */ nes::controller_1.read_d0_bit();
    result.w = nes::controller_1.read_d0_bit();
    result.s = nes::controller_1.read_d0_bit();
    result.a = nes::controller_1.read_d0_bit();
    result.d = nes::controller_1.read_d0_bit();
    return result;
  }

  // Just fill in enough of the notes to cover what I need.
  struct note_base {
    enum Octave_3 {
      C_3 = 0x1ab,
    };

    enum Octave_4 { A_SHARP_4 = 0x0ef, C_4 = 0x0d2 };

    enum Octave_5 { A_SHARP_5 = 0x07e};

    constexpr note_base(std::uint16_t period, std::uint8_t duration)
        : m_period{period}, m_duration{duration} {}

    std::uint16_t m_period;
    std::uint8_t m_duration;
  };

  struct note {};

  struct triangle_note : public note_base {

    constexpr triangle_note(std::uint16_t period, std::uint8_t duration,
                            TriangleChannel::Control control)
        : note_base{period, duration}, m_control{control} {}

    void play() const {
      if (m_period) {
        apu.triangle.set_period(m_period);
        apu.triangle.set_control(m_control);
        apu.set_frame_counter(APU::Mode_5_step | APU::InhibitIrq);
      }
      else{
        apu.triangle.set_control(m_control);
        apu.set_frame_counter(APU::Mode_5_step);
      }
    };

    TriangleChannel::Control m_control;
  };

  struct pulse_note : public note_base {

  };

  struct noise_note : public note_base {
    void play() const {
      apu.noise.set_mode_period(m_mode, m_period);
      apu.noise.set_control_volume(m_control, m_volume);
      if (m_volume) {
        apu.noise.set_length_counter(NoiseChannel::LengthCounterMax);
      }
      else
      {
        apu.noise.set_length_counter(NoiseChannel::LengthCounterMin);
      }
    }

    constexpr noise_note(NoiseChannel::Control control, NoiseChannel::Mode mode,
                         std::uint8_t volume, std::uint8_t period,
                         std::uint8_t duration)
        : note_base{period, duration}, m_control{control}, m_mode{mode}, m_volume{volume} {}

    NoiseChannel::Control m_control;
    NoiseChannel::Mode m_mode;
    std::uint8_t  m_volume : 4;
  };

  struct sounds {
    static constexpr triangle_note expose_sfx[] = {
        {triangle_note::A_SHARP_5, 10,
         TriangleChannel::Unmute | TriangleChannel::LengthCounterHalt},
        {triangle_note::C_4, 5,
         TriangleChannel::Unmute | TriangleChannel::LengthCounterHalt},
        {0, 1, TriangleChannel::LengthCounterHalt}};

    static constexpr noise_note shoot_sfx[] = {
        {NoiseChannel::LengthCounterHalt | NoiseChannel::ConstantVolume,
         NoiseChannel::None, 0xF, 0x5, 3},
        {NoiseChannel::LengthCounterHalt | NoiseChannel::ConstantVolume,
         NoiseChannel::None, 0x0, 0, 1}};
    static constexpr triangle_note flag_sfx[] = {
        {triangle_note::C_3, 13,
         TriangleChannel::Unmute | TriangleChannel::LengthCounterHalt},
        {0, 1, TriangleChannel::LengthCounterHalt}};
  };

  class music {
  public:
    static void update() {
      update_channel(triangle_channel);
      update_channel(noise_channel);
    }

    template <class NoteType, size_t N>
    static void play(std::uint8_t channel, bool loop,
                     const NoteType (&notes)[N]) {
      play(channel, loop, notes, notes + N);
    }

    static void play(std::uint8_t channel, bool loop, const note *begin,
                     const note *end) {}

    static void play(std::uint8_t, bool loop, const triangle_note *begin,
                     const triangle_note *end) {
      play_to_channel(loop, triangle_channel, begin, end);
    }

    static void play(std::uint8_t, bool loop, const noise_note *begin,
                     const noise_note *end) {
      play_to_channel(loop, noise_channel, begin, end);
    }

  private:
    template <class NoteType> struct channel_state {
      const NoteType *begin_note;
      const NoteType *current_note;
      const NoteType *end_note;
      std::uint8_t ticks_left;
      bool loop;

      void start_current_note() {
        if (current_note != end_note) {
          current_note->play();
          ticks_left = current_note->m_duration;
        }
      }
    };

    template <class NoteType>
    static void play_to_channel(bool loop, channel_state<NoteType> &channel,
                                const NoteType *begin, const NoteType *end) {
      channel.begin_note = channel.current_note = begin;
      channel.end_note = end;
      channel.loop = loop;
      channel.start_current_note();
    }

    template <class ChannelType>
    static void update_channel(ChannelType &update_channel) {
      if (update_channel.ticks_left == 0) {
        if (update_channel.current_note == update_channel.end_note) {
          if (update_channel.loop) {
            update_channel.current_note = update_channel.begin_note;
          }
        } else {
          update_channel.current_note += 1;
        }

        update_channel.start_current_note();
      } else {
        update_channel.ticks_left -= 1;
      }
    }

    static channel_state<triangle_note> triangle_channel;
    static channel_state<noise_note> noise_channel;
  };

  inline music::channel_state<triangle_note> music::triangle_channel{};
  inline music::channel_state<noise_note> music::noise_channel{};

  static void audio_setup() { apu.init(); }

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

  inline const target::graphics::Palettes *target::graphics::next_palettes =
      nullptr;
  inline PPU::pointer target::graphics::window_base{PPU::NAME_TABLE_0};

  inline std::int8_t target::graphics::sprite::sprite_x_offset = 0;

  }// namespace target
}

namespace target = nes::target;

#endif
