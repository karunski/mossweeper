#pragma once

#ifndef NES_H
#define NES_H

#include <cstddef>
#include <ppu.h>
#include <string.h>

namespace nes {
  
  struct PPURegisters {
    volatile std::byte PPUCTRL;
    volatile std::byte PPUMASK;
    volatile std::byte PPUSTATUS;
    std::byte OAMADDR;
    std::byte OAMDATA;
    volatile std::byte PPUSCROLL;
    volatile std::byte PPUADDR;
    volatile std::byte PPUDATA;
  };

  static_assert(offsetof(PPURegisters, PPUADDR) == 6);
  static_assert(offsetof(PPURegisters, PPUDATA) == 7);

  struct Color;

  class PPU : private PPURegisters {
  public:

    bool vblank() const volatile {
      return (PPUSTATUS & std::byte{0b10000000}) == std::byte{0b10000000};
    }

    std::byte status() const volatile { return PPUSTATUS; }

    enum render_bits : std::uint8_t {
      render_off = 0,
      enable_bg_column_0 = 0b00000010,
      enable_sprite_column_0 = 0b00000100,
      enable_bg = 0b00001000,
      enable_sprite = 0b00010000
    };

    void set_render_control(render_bits maskbits) volatile {
      PPUMASK = std::byte{maskbits};
    }

    struct reference {
      template<class T>
      T assign_byte(T b);

      template<class T>
      T operator=(T val) { return assign_byte(val); }

      std::uint16_t ptr;
    };

    struct pointer {
      reference operator*() const { return reference {ptr}; }

      std::uint16_t ptr;
    };

    template <class T> static void fill(pointer dest, T, size_t len);

    template <class T> static void copy(pointer dest, const T *src, std::uint8_t len);

    void set_ppu_address(pointer ptr) volatile {
      (void)PPUSTATUS;
      PPUADDR = static_cast<std::byte>(ptr.ptr >> 8);
      PPUADDR = static_cast<std::byte>(ptr.ptr & 0xff);
    }

    void store_data(std::byte b) volatile { PPUDATA = b; }

    void set_scroll(std::uint8_t x, std::uint8_t y) volatile {
      PPUSCROLL = static_cast<std::byte>(x);
      PPUSCROLL = static_cast<std::byte>(y);
    }

    static constexpr pointer PALETTE_BASE{0x3F00};
    static constexpr pointer PALETTE_BACKGROUND[4] = {0x3F01, 0x3F05, 0x3F09, 0x3F0D};
    static constexpr pointer PALETTE_SPRITE[4] = {0x3F11, 0x3F15, 0x3f19, 0x3f1D};
    static constexpr pointer NAME_TABLE_0{0x2000};
    static constexpr pointer ATTRIBUTE_TABLE_0{0x23C0};
  };

  PPU::render_bits operator|(PPU::render_bits left, PPU::render_bits right) {
    return static_cast<PPU::render_bits>(left | right);
  }

  inline auto &ppu = *(reinterpret_cast<volatile PPU *>(&::PPUCTRL));

  template<class T>
  T PPU::reference::assign_byte(T b) {
    static_assert(sizeof(T) == 1, "only byte width stores to PPUDATA");
    std::byte temp;
    memcpy(&temp, &b, 1);
    ppu.set_ppu_address(pointer{ptr});
    ppu.store_data(temp);
    return b;
  }

  template<class T>
  void PPU::fill(pointer dest, T val, size_t len) {
    static_assert(sizeof(T) == 1, "only byte width stores to PPUDATA");

    std::byte temp;
    memcpy(&temp, &val, 1);

    ppu.set_ppu_address(dest);
    for (size_t i = 0; i < len; i += 1) {
      ppu.store_data(temp);
    }
  }

  template<class T>
  void PPU::copy(pointer dest, const T * src, std::uint8_t len) {
    static_assert(sizeof(T) == 1, "only byte width stores to PPUDATA");

    ppu.set_ppu_address(dest);
    for (std::uint8_t i = 0; i < len; i += 1) {
      std::byte temp;
      memcpy(&temp, src+i, 1);
      ppu.store_data(temp);
    }
  }

  PPU::pointer operator+(const PPU::pointer & ptr, int offset) {
    return PPU::pointer{static_cast<uint16_t>(ptr.ptr + offset)};
  }

  struct Color {

    enum Luma {
      Dark = 0x00,
      Medium = 0x10,
      Light = 0x20,
      Pale = 0x30,
    };

    enum Chroma {
      Gray = 0, 
      Azure = 0x1,
      Blue = 0x2,
      Violet = 0x3,
      Magenta = 0x4,
      Rose = 0x5,
      Red = 0x6,
      Orange = 0x7,
      Yellow = 0x8,
      Chartreuse = 0x9,
      Green = 0xA,
      Spring = 0xB,
      Cyan = 0xC,
      Black = 0xD
    };

    constexpr Color(Luma luma, Chroma hue)
        : value{static_cast<std::uint8_t>(luma | hue)} {}
    constexpr Color() : Color{Dark, Black} {}

    std::uint8_t value;
  };

  inline auto get_vsync_wait() {
    return []() {
      while (!ppu.vblank()) {
      }
    };
  }

  struct ControllerPortRegisters {
    volatile std::byte io;
  };


  // Controller Port 1 reads the control port, and it has the latch control
  // for controlling when inputs are sampled.
  struct ControllerPort : private ControllerPortRegisters {
    void start_sample() volatile {
      io = std::byte{0b00000001};
    }

    void stop_sample() volatile {
      io = std::byte{0};
    }

    bool read_d0_bit() volatile {
      const auto byte = io;
      return (byte & std::byte{0b00000001}) ==
               std::byte{0b00000001}; // check LSB, and invert result
    }
  };

  inline auto &controller_1 =
      *(reinterpret_cast<volatile ControllerPort *>(0x4016));
  inline auto &controller_2 =
      *(reinterpret_cast<volatile ControllerPort *>(0x4017));
}

#endif
