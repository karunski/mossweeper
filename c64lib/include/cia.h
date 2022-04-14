#pragma once

#ifndef C64_CIA_H
#define C64_CIA_H

#include <cstddef>
#include <cstdint>

namespace c64 {
struct CIARegisters {
  volatile std::byte data_port_a;
  volatile std::byte data_port_b;

  std::byte data_direction_a;
  std::byte data_direction_b;

  std::byte m_todo[0x09];
  volatile std::byte icr;
};

static_assert(offsetof(CIARegisters, data_port_a) == 0);
static_assert(offsetof(CIARegisters, data_port_b) == 0x01);
static_assert(offsetof(CIARegisters, data_direction_a) == 0x02);
static_assert(offsetof(CIARegisters, data_direction_b) == 0x03);
static_assert(offsetof(CIARegisters, icr) == 0x0d);

extern volatile std::byte dead_store;

class CIA : private CIARegisters {
public:
  struct InterruptSet {
    enum SetMode { CLEAR = 0, SET = 1 };

    constexpr InterruptSet(SetMode mode, bool timer_a, bool timer_b, bool alarm,
                           bool xfer, bool flag)
        : bits{static_cast<std::byte>(timer_a) |
               (static_cast<std::byte>(timer_b) << 1) |
               (static_cast<std::byte>(alarm) << 2) |
               (static_cast<std::byte>(xfer) << 3) |
               (static_cast<std::byte>(flag) << 4) |
               (static_cast<std::byte>(mode) << 7)} {}

    constexpr std::byte get() const { return bits; }

  private:
    std::byte bits;
  };

  void interrupt_control(InterruptSet interrupt_set) volatile {
    icr = interrupt_set.get();
  }

  std::byte interrupt_status() volatile {
    std::byte res = icr;
    dead_store = res;
    return res;
  }

  using CIARegisters::data_port_a;
  using CIARegisters::data_port_b;

  using CIARegisters::data_direction_a;
  using CIARegisters::data_direction_b;
private:
};

class CIA2 : public CIA
{
public:
  enum class vic_bank : std::uint8_t {
    NO_0 = 0b00000011,
    NO_1 = 0b00000010,
    NO_2 = 0b00000001,
    NO_3 = 0b00000000
  };

  template <vic_bank Bank> struct VicBankBaseAddress {
    static constexpr std::uint16_t value =
        static_cast<std::uint16_t>(0x4000) * (~static_cast<std::uint16_t>(Bank) & 0b11);
  };

  void set_vic_bank(vic_bank bank)
  {
    data_direction_a = (data_direction_a | std::byte{0b00000011});
    data_port_a = (data_port_a & std::byte{0b11111100}) | static_cast<std::byte>(bank);
  }
};



inline auto &cia1 = *(reinterpret_cast<CIA *>(0xDC00));
inline auto &cia2 = *(reinterpret_cast<CIA2 *>(0xDD00));
}

#endif // C64_CIA_H
