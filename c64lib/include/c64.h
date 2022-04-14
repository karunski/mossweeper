#pragma once

#ifndef C64_H
#define C64_H

#include <cstddef>
#include <cstdint>

#include "cia.h"

extern "C" void my_irq();

namespace std {
template <class T, std::size_t N> constexpr std::size_t size(const T (&)[N]) {
  return N;
}
} // namespace std

namespace c64
{
enum class ScreenCode : std::uint8_t {
  AT,
  A,
  B,
  C,
  D,
  E,
  F,
  G,
  H,
  I,
  J,
  K,
  L,
  M,
  N,
  O,
  P,
  Q,
  R,
  S,
  T,
  U,
  V,
  W,
  X,
  Y,
  Z,
  LEFT_SQUARE_BRACKET,
  GB_POUND,
  RIGHT_SQUARE_BRACKET,
  UP,
  LEFT,
  SPACE,
  EXCLAMATION,
  DOUBLE_QUOTE,
  HASHTAG,
  DOLLAR,
  PERCENT,
  AMPERSAND,
  APOSTROPHE,
  LEFT_PAREN,
  RIGHT_PAREN,
  ASTERISK,
  PLUS,
  COMMA,
  DASH,
  PERIOD,
  SLASH,
  NUMBER_0,
  NUMBER_1,
  NUMBER_2,
  NUMBER_3,
  NUMBER_4,
  NUMBER_5,
  NUMBER_6,
  NUMBER_7,
  NUMBER_8,
  NUMBER_9,
  COLON,
  SEMICOLON,
  LESS_THAN,
  EQUAL,
  GREATER_THAN,
  QUESTION
};

constexpr ScreenCode to_screen_code(char c) {
  switch ((c >> 5) & 0x3) {

  case 1:
    return static_cast<ScreenCode>(c);
  case 2:
    return static_cast<ScreenCode>(c - 64);
  case 3:
    return static_cast<ScreenCode>(c - 96);
  default:
    return ScreenCode::SPACE;
  }
}

enum class ColorCode : std::uint8_t {
  BLACK = 0,
  WHITE = 1,
  RED = 2,
  CYAN = 3,
  VIOLET = 4,
  GREEN = 5,
  BLUE = 6,
  YELLOW = 7,
  ORANGE = 8,
  BROWN = 9,
  LIGHT_RED = 10,
  DARK_GREY = 11,
  GREY = 12,
  LIGHT_GREEN = 13,
  LIGHT_BLUE = 14,
  LIGHT_GREY = 15
};

struct Sprite {
  std::byte bit_data[63];

  struct Info {
    enum Mode { STANDARD = 0x00, MULTICOLOR = 0x80 };

    Mode mode() const { return static_cast<Mode>(mode_byte & std::byte{0x80}); }
    c64::ColorCode sprite_color() const {
      return static_cast<c64::ColorCode>(mode_byte & std::byte{0xF});
    }

  private:
    std::byte mode_byte;
  } mode;
};

class SpritePointer
{
public:
  SpritePointer & operator=(const Sprite & spr)
  {
    m_val = static_cast<std::uint8_t>(reinterpret_cast<std::uint16_t>(&spr) / 64);
    return *this;
  }

  explicit operator unsigned() const { return m_val; }

private:
  std::uint8_t m_val;
};

class ScreenRAM {
public:
  ScreenCode *data() { return &m_chars[0][0]; }

  ScreenCode &at(std::uint8_t x, std::uint8_t y) { return m_chars[y][x]; }

  SpritePointer & sprite_ptr(std::uint8_t sprite) { return m_spriteptr[sprite]; }

private:
  ScreenCode m_chars[25][40];
  std::uint8_t m_unused[0x10];
  SpritePointer m_spriteptr[8];
};

static_assert(sizeof(ScreenRAM) == (40 * 25)+16+8);

class ColorRAM {
public:
  ColorCode *data() { return &m_colors[0][0]; }

  ColorCode &at(std::uint8_t x, std::uint8_t y) { return m_colors[y][x]; }

private:
  ColorCode m_colors[25][40];
};

struct VicIIInterruptStatus {
  bool raster() const volatile {
    return (m_val & std::byte{0x1}) != std::byte{0};
  }
  bool mbc() const volatile { return (m_val & std::byte{0x2}) != std::byte{0}; }
  bool mmc() const volatile { return (m_val & std::byte{0x4}) != std::byte{0}; }
  bool lp() const volatile { return (m_val & std::byte{0x8}) != std::byte{0}; }

private:
  std::byte m_val;
};

struct SpriteBitset
{
  void set(std::uint8_t bit, bool value) {
    m_bits = (m_bits & ~(std::byte(1) << bit)) | (std::byte(value) << bit);
  }

  template<std::uint8_t Bit>
  void set(bool value) {
    m_bits = (m_bits & ~(std::byte(1) << Bit)) | (std::byte(value) << Bit);
  }

  bool test(std::uint8_t bit) const {
    return static_cast<bool>((m_bits >> bit) & std::byte(1));
  }

  void clear() { m_bits = std::byte{0}; }

  std::byte m_bits;
};

struct VicIIRegisters {
  struct SpritePos {
    std::uint8_t x, y;
  };
  SpritePos m_pos[8];
  SpriteBitset sprite_x_msb;
  volatile std::byte cr1;
  volatile std::byte raster;
  std::byte lpx;
  std::byte lpy;
  SpriteBitset sprite_enable;
  std::byte cr2;
  SpriteBitset sprite_y_expansion;
  std::byte memory_setup;
  volatile VicIIInterruptStatus interrupt_status;
  std::byte interrupt_enable;
  SpriteBitset sprite_data_priority;
  SpriteBitset sprite_multicolor_enable;
  SpriteBitset sprite_x_expansion;
  std::byte sprite_collision;
  std::byte sprite_bg_collision;
  ColorCode border_color;
  ColorCode background_color[4];
  ColorCode sprite_multicolor[2];
  ColorCode sprite_color[8];
};

static_assert(offsetof(VicIIRegisters, cr1) == 0x11);
static_assert(offsetof(VicIIRegisters, raster) == 0x12);
static_assert(offsetof(VicIIRegisters, sprite_enable) == 0x15);
static_assert(offsetof(VicIIRegisters, cr2) == 0x16);
static_assert(offsetof(VicIIRegisters, sprite_y_expansion) == 0x17);
static_assert(offsetof(VicIIRegisters, memory_setup) == 0x18);
static_assert(offsetof(VicIIRegisters, interrupt_status) == 0x19);
static_assert(offsetof(VicIIRegisters, interrupt_enable) == 0x1a);
static_assert(offsetof(VicIIRegisters, sprite_data_priority) == 0x1b);
static_assert(offsetof(VicIIRegisters, sprite_multicolor_enable) == 0x1c);
static_assert(offsetof(VicIIRegisters, sprite_x_expansion) == 0x1d);
static_assert(offsetof(VicIIRegisters, border_color) == 0x20);
static_assert(offsetof(VicIIRegisters, background_color) == 0x21);
static_assert(offsetof(VicIIRegisters, sprite_multicolor) == 0x25);
static_assert(offsetof(VicIIRegisters, sprite_color) == 0x27);

class VicII : private VicIIRegisters {
public:
  using VicIIRegisters::border_color;
  using VicIIRegisters::background_color;

  struct InterruptSet {
    constexpr InterruptSet(bool light_pen, bool sprite_collision,
                           bool background_collision, bool raster)
        : bits{(static_cast<std::byte>(light_pen) << 3) |
               (static_cast<std::byte>(sprite_collision) << 2) |
               (static_cast<std::byte>(background_collision) << 1) |
               static_cast<std::byte>(raster)} {}

    constexpr std::byte get() const { return bits; }

  private:
    std::byte bits;
  };

  void enable_interrupts(const InterruptSet en) volatile {
    interrupt_enable = en.get();
  }

  void set_raster(std::uint16_t rs) volatile {
    raster = std::byte(rs & 0xFF);
    cr1 = (std::byte(rs >> 8) & std::byte{0x80}) | (cr1 & std::byte{0x7f});
  }

  std::uint16_t get_raster() volatile {
    const auto raster_val = raster;
    const auto cr1_val = cr1;
    return static_cast<std::uint16_t>(raster_val) | (static_cast<std::uint16_t>(cr1_val & std::byte{0x80}) << 1);
  }

  using VicIIRegisters::cr1;
  using VicIIRegisters::cr2;
  using VicIIRegisters::memory_setup;

  using VicIIRegisters::sprite_enable;
  using VicIIRegisters::sprite_multicolor_enable;
  using VicIIRegisters::sprite_data_priority;
  using VicIIRegisters::sprite_multicolor;
  using VicIIRegisters::sprite_color;
  using VicIIRegisters::sprite_y_expansion;
  using VicIIRegisters::sprite_x_expansion;

  void set_sprite_pos(std::uint8_t sprite, std::uint16_t x, std::uint8_t y)
  {
    sprite_x_msb.set(sprite, (x >> 8) & 0x1);
    m_pos[sprite].x = x & 0xFF;
    m_pos[sprite].y = y;
  }

  template<std::uint8_t SpriteN>
  void set_sprite_pos(std::uint16_t x, std::uint8_t y)
  {
    sprite_x_msb.set<SpriteN>((x >> 8) & 0x1);
    m_pos[SpriteN].x = x & 0xFF;
    m_pos[SpriteN].y = y;
  }

  enum CharMemoryOffset {
    TEXT_0000 = 0b00000000,
    TEXT_0800 = 0b00000010,
    TEXT_1000 = 0b00000100,
    TEXT_1800 = 0b00000110,
    TEXT_2000 = 0b00001000,
    TEXT_2800 = 0b00001010,
    TEXT_3000 = 0b00001100,
    TEXT_3800 = 0b00001110
  };

  template<CharMemoryOffset CharMemoryBits>
  struct CharMemoryOffsetInt {
    static constexpr std::uint16_t value =
        static_cast<std::uint16_t>(CharMemoryBits) << 10;
  };

  enum VideoMatrixOffset {
    VIDEO_OFFSET_0000 = 0b00000000,
    VIDEO_OFFSET_0400 = 0b00010000,
    VIDEO_OFFSET_0800 = 0b00100000,
    VIDEO_OFFSET_0C00 = 0b00110000,
    VIDEO_OFFSET_1000 = 0b01000000,
    VIDEO_OFFSET_1400 = 0b01010000,
    VIDEO_OFFSET_1800 = 0b01100000,
    VIDEO_OFFSET_1C00 = 0b01110000,
    VIDEO_OFFSET_2000 = 0b10000000,
    VIDEO_OFFSET_2400 = 0b10010000,
    VIDEO_OFFSET_2800 = 0b10100000,
    VIDEO_OFFSET_2C00 = 0b10110000,
    VIDEO_OFFSET_3000 = 0b11000000,
    VIDEO_OFFSET_3400 = 0b11010000,
    VIDEO_OFFSET_3800 = 0b11100000,
    VIDEO_OFFSET_3C00 = 0b11110000,
  };

  template <VideoMatrixOffset VideoMatrixBits>
  struct VideoMatrixOffsetInt {
    static constexpr std::uint16_t value = static_cast<std::uint16_t>(VideoMatrixBits) << 6;
  };

  void setup_char_memory(CharMemoryOffset char_offset) {
    memory_setup = (memory_setup & std::byte(0xF0)) | std::byte(char_offset);
  }

  void setup_memory(VideoMatrixOffset video_offset, CharMemoryOffset char_offset)
  {
    memory_setup = std::byte(video_offset) | std::byte(char_offset);
  }

  void set_multi_color_mode(bool enable) {
    cr2 = enable ? (cr2 | (std::byte(1) << 4)) : (cr2 & ~(std::byte(1) << 4));
  }
};

struct CharRAM {
  std::byte data[2048];
};

template <CIA2::vic_bank Bank, VicII::VideoMatrixOffset VideoMatrixSetting,
          VicII::CharMemoryOffset CharMemorySetting>
struct DisplayAddr {
  static constexpr auto vic_base_setting = Bank;
  static constexpr auto screen_setting = VideoMatrixSetting;
  static constexpr auto char_data_setting = CharMemorySetting;
  static constexpr std::uint16_t screen_int_addr =
      CIA2::VicBankBaseAddress<Bank>::value +
      VicII::VideoMatrixOffsetInt<VideoMatrixSetting>::value;
  static constexpr std::uint16_t char_int_addr =
      CIA2::VicBankBaseAddress<Bank>::value +
      VicII::CharMemoryOffsetInt<CharMemorySetting>::value;
  static ScreenRAM &screen;
  static CharRAM &chars;
};

static_assert(CIA2::VicBankBaseAddress<CIA2::vic_bank::NO_3>::value == 0xC000);
static_assert(DisplayAddr<CIA2::vic_bank::NO_3,
                          VicII::VIDEO_OFFSET_0000, VicII::TEXT_0800>::screen_int_addr == 0xC000);
static_assert(DisplayAddr<CIA2::vic_bank::NO_3, VicII::VIDEO_OFFSET_0000,
                          VicII::TEXT_0800>::char_int_addr == 0xC800);

template <CIA2::vic_bank Bank, VicII::VideoMatrixOffset VideoMatrixSetting,
          VicII::CharMemoryOffset CharMemorySetting>
ScreenRAM &DisplayAddr<Bank, VideoMatrixSetting, CharMemorySetting>::screen =
    *reinterpret_cast<ScreenRAM *>(screen_int_addr);

template <CIA2::vic_bank Bank, VicII::VideoMatrixOffset VideoMatrixSetting,
          VicII::CharMemoryOffset CharMemorySetting>
CharRAM &DisplayAddr<Bank, VideoMatrixSetting,
                        CharMemorySetting>::chars =
    *reinterpret_cast<CharRAM *>(char_int_addr);

inline auto &color_ram = *(reinterpret_cast<ColorRAM *>(0xD800));
inline auto &vic_ii = *(reinterpret_cast<VicII *>(0xD000));


struct VsyncWaitFunc
{
  void operator()() const;
};

VsyncWaitFunc get_vsync_wait();

struct ScopedInterruptDisable {

  inline ScopedInterruptDisable() { asm volatile("sei" : : :); }

  inline ~ScopedInterruptDisable() { asm volatile("cli" : : :); }
};

}

#endif
