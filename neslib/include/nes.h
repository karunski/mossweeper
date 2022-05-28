#pragma once

#ifndef NES_H
#define NES_H

#include <cstddef>
#include <ppu.h>
#include <string.h>

namespace nes {

  enum palette_index : std::uint8_t {
    PALETTE_0 = 0,
    PALETTE_1 = 1,
    PALETTE_2 = 2,
    PALETTE_3 = 3
  };

  struct oam_entry {
    std::uint8_t y;
    std::uint8_t tile_index;
    std::byte    attributes;
    std::uint8_t x;
  };

  static_assert(sizeof(oam_entry) == 4);
  static_assert(offsetof(oam_entry, x) == 3);

  class alignas(256) OAM {
  public:
    using slot_t = std::uint8_t;

    void set_sprite_position(slot_t slot, std::uint8_t x, std::uint8_t y) {
      auto & entry = entries[slot];
      entry.x = x;
      entry.y = y;
    }

    void set_sprite_tile_index(slot_t slot, std::uint8_t tile) {
      entries[slot].tile_index = tile;
    }

    enum sprite_attributes : std::uint8_t {
      no_attributes = 0,
      behind_background = 0b00100000,
      flip_horizontally = 0b01000000,
      flip_vertically = 0b10000000
    };

    void set_sprite_attributes(slot_t slot, palette_index palette, sprite_attributes attr) {
      entries[slot].attributes = static_cast<std::byte>(
          static_cast<std::uint8_t>(palette) | static_cast<std::uint8_t>(attr));
    }

    void hide_all_sprites() {
      for (auto & entry : entries) {
        entry.y = 0xff;
      }
    }

  private:
    oam_entry entries[64];
  };

  OAM::sprite_attributes operator|(OAM::sprite_attributes left, OAM::sprite_attributes right) {
    return static_cast<OAM::sprite_attributes>(static_cast<std::uint8_t>(left) | static_cast<std::uint8_t>(right));
  }

  struct oam_dma_register {
    volatile std::uint8_t oam_addr_hi;
  };

  class OAMDMA : private oam_dma_register {
  public:
    void load_oam(const OAM & oam) volatile {
      oam_addr_hi = static_cast<std::uint8_t>(reinterpret_cast<std::uint16_t>(&oam) >> 8);
    }
  };

  inline auto &oam_dma = *(reinterpret_cast<volatile OAMDMA *>(0x4014));

  struct PPURegisters {
    volatile std::byte PPUCTRL;
    volatile std::byte PPUMASK;
    volatile std::byte PPUSTATUS;
    volatile std::byte OAMADDR;
    volatile std::byte OAMDATA;
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
      PPUMASK = static_cast<std::byte>(maskbits);
    }

    struct reference {
      template<class T>
      T assign_byte(T b);

      template<class T>
      T operator=(T val) { return assign_byte(val); }

      template<class T>
      T fetch_byte() const;

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
    std::byte load_data() volatile const {
      return PPUDATA;
    }

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
    return static_cast<PPU::render_bits>(static_cast<std::uint8_t>(left) | static_cast<std::uint8_t>(right));
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
  T PPU::reference::fetch_byte() const {
    static_assert(sizeof(T) == 1, "only byte width loads from PPUDATA");
    ppu.set_ppu_address(pointer{ptr});
    ppu.load_data(); // discard first vram read after setting address.
    std::byte temp = ppu.load_data();
    T b;
    memcpy(&b, &temp, 1);
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
