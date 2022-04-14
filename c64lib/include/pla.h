#pragma once

#ifndef C64_PLA_H
#define C64_PLA_H

#include <cstdint>

namespace c64 {
  struct PLA;
}

extern "C" {
extern c64::PLA pla;
}

namespace c64 {

inline auto &pla = ::pla;

struct pla_registers {
  std::byte data_direction;
  volatile std::byte data_port;
};

struct PLA : private pla_registers {
  enum cpu_control : std::uint8_t {
    CHAREN  = 0b00000100,
    HIRAM   = 0b00000010,
    LORAM   = 0b00000001,
    MODE_31 = 0b00000111, // BASIC, I/O, KERNAL
    MODE_30 = 0b00000110, // I/O, KERNAL
    MODE_29 = 0b00000101, // I/O
    MODE_28 = 0b00000100, // just ram
    MODE_27 = 0b00000011, // BASIC, CHAR ROM, KERNAL
    MODE_26 = 0b00000010, // CHAR ROM, KERNAL
    MODE_25 = 0b00000001, // CHAR ROM
  };

  void set_cpu_lines(cpu_control mode) {
    data_direction = (data_direction | std::byte{0b00000111});
    data_port = (data_port & std::byte{0b0011100}) | static_cast<std::byte>(mode);
  }

  cpu_control get_cpu_lines() const {
    return static_cast<cpu_control>(data_port & std::byte{0b00000111});
  }

  class BankSwitchScope {
  public:
    explicit BankSwitchScope(cpu_control new_mode)
        : m_saved(pla.get_cpu_lines()) {
      pla.set_cpu_lines(new_mode);
    }

    ~BankSwitchScope() { pla.set_cpu_lines(m_saved); }

  private:
    const cpu_control m_saved;
  };
};
}

#endif // C64_PLA_H
