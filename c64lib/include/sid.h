#pragma once

#ifndef C64_SID_H
#define C64_SID_H

#include <cstddef>
#include <cstdint>
#include <string.h>

namespace c64
{
  struct SIDVoiceRegisters
  {
    std::byte frequency[2]; // low/high
    std::byte pulse_wave_duty_cycle[2]; // low/high, 12 bits
    volatile std::byte    control;
    std::byte attack_delay;
    std::byte sustain_release;
  };

  static_assert(offsetof(SIDVoiceRegisters, control) == 4);
  static_assert(offsetof(SIDVoiceRegisters, attack_delay) == 5);
  static_assert(offsetof(SIDVoiceRegisters, sustain_release) == 6);

  struct Nibble {
    std::uint8_t val : 4;
  };

  struct NibblePair {
    std::uint8_t high : 4;
    std::uint8_t low : 4;

    std::byte combine() const {
      return (std::byte{high} << 4) | std::byte{low};
    }
  };

  static_assert(sizeof(NibblePair) == 1);

  class SIDVoice : private SIDVoiceRegisters {
  public:
    void set_frequency(std::uint16_t freq) {
      memcpy(&frequency, &freq, sizeof(frequency));
    }

    enum ControlFlags : std::uint8_t
    {
      gate = 0b00000001,
      syncronize = 0b00000010,
      ring = 0b00000100,
      test = 0b00001000,
      triangle = 0b00010000,
      sawtooth = 0b00100000,
      pulse    = 0b01000000,
      noise    = 0b10000000
    };

    void set_control(ControlFlags control_flags) {
      control = std::byte{control_flags};
    }

    void clear() {
      *this = SIDVoice{};
    }

    void set_attack_decay(Nibble attack, Nibble decay) {
      attack_delay = NibblePair{attack.val, decay.val}.combine();
    }

    void set_sustain_release(Nibble sustain, Nibble release) {
      sustain_release = NibblePair{sustain.val, release.val}.combine();
    }
  };

  constexpr SIDVoice::ControlFlags operator|(SIDVoice::ControlFlags left, SIDVoice::ControlFlags right) {
    return static_cast<SIDVoice::ControlFlags>(static_cast<std::uint8_t>(left) | static_cast<std::uint8_t>(right));
  }

  struct SIDRegisters
  {
    SIDVoice voices[3];
    std::byte filter_cutoff_freq[2];
    std::byte filter_resonance_routing;
    std::byte filter_volume;
    std::uint8_t paddle_x;
    std::uint8_t paddle_y;
    volatile std::uint8_t oscillator_3;
    std::uint8_t envelope_3;
  };

  static_assert(offsetof(SIDRegisters, voices[1]) == 7);
  static_assert(offsetof(SIDRegisters, voices[2]) == 0xe);
  static_assert(offsetof(SIDRegisters, oscillator_3) == 0x1b);
  static_assert(offsetof(SIDRegisters, filter_volume) == 0x18);

  class SID : private SIDRegisters {
  public:
    using SIDRegisters::voices;
    using SIDRegisters::oscillator_3;

    void clear() {
      for (auto &voice : voices) {
        voice.clear();
      }
      memset(filter_cutoff_freq, 0, sizeof(filter_cutoff_freq));
      filter_resonance_routing = std::byte{};
      filter_volume = std::byte{};
    }

    enum VolumeBits : std::uint8_t
    {
      no_volume_bits = 0,
      low_pass     = 0b00010000,
      band_pass    = 0b00100000,
      high_pass    = 0b01000000,
      mute_voice_3 = 0b10000000
    };

    void set_volume(Nibble vol, VolumeBits flags) {
      filter_volume = std::byte{vol.val} | std::byte{flags};
    }
  };

  inline auto &sid = *(reinterpret_cast<SID *>(0xD400));
}
#endif
