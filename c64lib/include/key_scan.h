#pragma once

#ifndef C64_KEY_SCAN_H
#define C64_KEY_SCAN_H

#include <cstddef>
#include <cstdint>

#include "cia.h"

namespace c64
{
constexpr std::byte KEYBOARD_MATRIX_ROW[] = {
    std::byte(0xFE),        std::byte(0xFD),        ~std::byte(0b00000100),
    ~std::byte(0b00001000), ~std::byte(0b00010000), ~std::byte(0b00100000),
    ~std::byte(0b01000000), ~std::byte(0b10000000)};

template <std::uint8_t RowNum> class KeyScanRowBase {
protected:
  KeyScanRowBase() : m_bits{fetch<KEYBOARD_MATRIX_ROW[RowNum]>()} {}

  template <std::uint8_t bit> constexpr bool test_key() const {
    return (m_bits & (std::byte(1) << bit)) == std::byte{0};
  }

  template <std::byte RowMatrix> static std::byte fetch() {
    cia1.data_port_a = RowMatrix;
    return c64::cia1.data_port_b;
  }

  const std::byte m_bits;
};

template <std::uint8_t Row> class KeyScan;

template <> struct KeyScan<0> : private KeyScanRowBase<0> {
  using KeyScanRowBase<0>::KeyScanRowBase;

public:
  bool KEY_DEL() const { return test_key<0>(); }
  bool KEY_RETURN() const { return test_key<1>(); }
  bool KEY_CURSOR_RIGHT() const { return test_key<2>(); }
  bool KEY_F7() const { return test_key<3>(); }
  bool KEY_F1() const { return test_key<4>(); }
  bool KEY_F3() const { return test_key<5>(); }
  bool KEY_F5() const { return test_key<6>(); }
  bool KEY_CURSOR_DOWN() const { return test_key<7>(); }
};

template <> struct KeyScan<1> : private KeyScanRowBase<1> {

  using KeyScanRowBase<1>::KeyScanRowBase;

public:
  bool KEY_3() const { return test_key<0>(); }
  bool KEY_W() const { return test_key<1>(); }
  bool KEY_A() const { return test_key<2>(); }
  bool KEY_4() const { return test_key<3>(); }
  bool KEY_Z() const { return test_key<4>(); }
  bool KEY_S() const { return test_key<5>(); }
  bool KEY_E() const { return test_key<6>(); }
  bool KEY_LEFT_SHIFT() const { return test_key<7>(); }
};

template <> struct KeyScan<2> : private KeyScanRowBase<2> {

  using KeyScanRowBase<2>::KeyScanRowBase;

public:
  bool KEY_5() const { return test_key<0>(); }
  bool KEY_R() const { return test_key<1>(); }
  bool KEY_D() const { return test_key<2>(); }
  bool KEY_6() const { return test_key<3>(); }
  bool KEY_C() const { return test_key<4>(); }
  bool KEY_F() const { return test_key<5>(); }
  bool KEY_T() const { return test_key<6>(); }
  bool KEY_X() const { return test_key<7>(); }
};

template <> struct KeyScan<3> : private KeyScanRowBase<3> {

  using KeyScanRowBase<3>::KeyScanRowBase;

public:
  bool KEY_7() const { return test_key<0>(); }
  bool KEY_Y() const { return test_key<1>(); }
  bool KEY_G() const { return test_key<2>(); }
  bool KEY_8() const { return test_key<3>(); }
  bool KEY_B() const { return test_key<4>(); }
  bool KEY_H() const { return test_key<5>(); }
  bool KEY_U() const { return test_key<6>(); }
  bool KEY_V() const { return test_key<7>(); }
};

template <> struct KeyScan<4> : private KeyScanRowBase<4> {

  using KeyScanRowBase<4>::KeyScanRowBase;

public:
  bool KEY_9() const { return test_key<0>(); }
  bool KEY_I() const { return test_key<1>(); }
  bool KEY_J() const { return test_key<2>(); }
  bool KEY_0() const { return test_key<3>(); }
  bool KEY_M() const { return test_key<4>(); }
  bool KEY_K() const { return test_key<5>(); }
  bool KEY_O() const { return test_key<6>(); }
  bool KEY_N() const { return test_key<7>(); }
};

template <> struct KeyScan<5> : private KeyScanRowBase<5> {

  using KeyScanRowBase<5>::KeyScanRowBase;

public:
  bool KEY_PLUS() const { return test_key<0>(); }
  bool KEY_P() const { return test_key<1>(); }
  bool KEY_L() const { return test_key<2>(); }
  bool KEY_DASH() const { return test_key<3>(); }
  bool KEY_PERIOD() const { return test_key<4>(); }
  bool KEY_COLON() const { return test_key<5>(); }
  bool KEY_AT() const { return test_key<6>(); }
};

template <> struct KeyScan<6> : private KeyScanRowBase<6> {

  using KeyScanRowBase<6>::KeyScanRowBase;

public:
  bool KEY_POUND_STERLING() const { return test_key<0>(); }
  bool KEY_ASTERISK() const { return test_key<1>(); }
  bool KEY_SEMICOLON() const { return test_key<2>(); }
  bool KEY_HOME() const { return test_key<3>(); }
  bool KEY_RIGHT_SHIFT() const { return test_key<4>(); }
  bool KEY_EQUAL() const { return test_key<5>(); }
  bool KEY_CARAT() const { return test_key<6>(); }
  bool KEY_SLASH() const { return test_key<7>(); }
};

template <> class KeyScan<7> : private KeyScanRowBase<7> {

  using KeyScanRowBase<7>::KeyScanRowBase;

public:
  bool KEY_SPACE() const { return test_key<4>(); }
  bool KEY_Q() const { return test_key<6>(); }
  bool KEY_2() const { return test_key<3>(); }
  bool KEY_1() const { return test_key<0>(); }
};

class KeyScanner {
public:
  KeyScanner() {
    cia1.data_direction_a = std::byte{0b11111111};
    cia1.data_direction_b = std::byte{0};
  }

  template <std::uint8_t RowNum> auto Row() const { return KeyScan<RowNum>{}; }
};

class JoyScanner {
public:
  JoyScanner() {
    cia1.data_direction_a = std::byte{0b11111111};
    cia1.data_direction_b = std::byte{0b11111111};
  }

  enum class JoyScan : std::uint8_t {
    CLEAR  = 0b00000000,
    UP     = 0b00000001,
    DOWN   = 0b00000010,
    LEFT   = 0b00000100,
    RIGHT  = 0b00001000,
    BUTTON = 0b00010000
  };

  struct JoyStickState {
    JoyScan joystick_a;
    JoyScan joystick_b;
  };

  JoyStickState scan_joysticks() const {
    cia1.data_port_a = std::byte{0xFF};
    cia1.data_port_b = std::byte{0xFF};
    return {
      static_cast<JoyScan>(~cia1.data_port_b),
          static_cast<JoyScan>(~cia1.data_port_a)};
  }
};

bool operator&(const JoyScanner::JoyScan & left, const JoyScanner::JoyScan & right) {
  return (static_cast<std::byte>(left) & static_cast<std::byte>(right)) != std::byte{0};
}

} // namespace c64
#endif // C64_KEY_SCAN_H
