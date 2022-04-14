#pragma once

#ifndef MUSIC_PLAYER_H
#define MUSIC_PLAYER_H

#include <sid.h>
#include <cstdint>

namespace c64 {

struct Note {
  enum Octave_0 : std::uint16_t {
    C_0 = 268,
    C_SHARP_0 = 284,
    D_0 = 301,
    D_SHARP_0 = 318,
    E_0 = 337,
    F_0 = 358,
    F_SHARP_0 = 379,
    G_0 = 401,
    G_SHARP_0 = 425,
    A_0 = 451,
    A_SHARP_0 = 477,
    B_0 = 506
  };

  enum Octave_1 : std::uint16_t {
    C_1 = 536,
    C_SHARP_1 = 568,
    D_1 = 602,
    D_SHARP_1 = 637,
    E_1 = 675,
    F_1 = 716,
    F_SHARP_1 = 758,
    G_1 = 803,
    G_SHARP_1 = 851,
    A_1 = 902,
    A_SHARP_1 = 955,
    B_1 = 1012,
  };

  enum Octave_2 : std::uint16_t {
    C_2 = 1072,
    C_SHARP_2 = 1136,
    D_2 = 1204,
    D_SHARP_2 = 1275,
    E_2 = 1351,
    F_2 = 1432,
    F_SHARP_2 = 1517,
    G_2 = 1607,
    G_SHARP_2 = 1703,
    A_2 = 1804,
    A_SHARP_2 = 1911,
    B_2 = 2025
  };

  enum Octave_3 : std::uint16_t {
    C_3 = 2145,
    C_SHARP_3 = 2273,
    D_3 = 2408,
    D_SHARP_3 = 2551,
    E_3 = 2703,
    F_3 = 2864,
    F_SHARP_3 = 3034,
    G_3 = 3215,
    G_SHARP_3 = 3406,
    A_3 = 3608,
    A_SHARP_3 = 3823,
    B_3 = 4050
  };

  enum Octave_4 : std::uint16_t {
    C_4 = 4291,
    C_SHARP_4 = 4547,
    D_4 = 4817,
    D_SHARP_4 = 5103,
    E_4 = 5407,
    F_4 = 5728,
    F_SHARP_4 = 6069,
    G_4 = 6430,
    G_SHARP_4 = 6812,
    A_4 = 7271,
    A_SHARP_4 = 7647,
    B_4 = 8101
  };

  enum Octave_5 : std::uint16_t {
    C_5 = 8583,

  };

  void play(std::uint8_t channel) const;

  constexpr Note(SIDVoice::ControlFlags flags, std::uint16_t freq,
                 std::uint8_t duration)
      : m_flags{flags}, m_freq{freq}, m_duration{duration} {}

  SIDVoice::ControlFlags m_flags;
  std::uint16_t m_freq;
  std::uint8_t m_duration;
};

class MusicPlayer {
public:
  static void update();

  template<size_t N>
  static void play(std::uint8_t channel, bool loop, const Note (&notes)[N]) {
    play(channel, loop, notes, notes+N);
  }

  static void play(std::uint8_t channel, bool loop, const Note * begin, const Note * end);

private:


  struct wave_channel {
    std::uint8_t channel;
    const Note * begin_note;
    const Note * current_note;
    const Note * end_note;
    std::uint8_t ticks_left;
    bool loop;

    void start_current_note();
  };
  static wave_channel channels[3];

};


} // namespace c64
#endif
