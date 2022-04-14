#include "music_player.h"

namespace c64 {


void Note::play(std::uint8_t channel) const {
  sid.voices[channel].set_frequency(m_freq);
  sid.voices[channel].set_control(m_flags);
}

MusicPlayer::wave_channel MusicPlayer::channels[3] = {};

void MusicPlayer::wave_channel::start_current_note() {
  if (current_note != end_note) {
    current_note->play(channel);
    ticks_left = current_note->m_duration;
  }
}

void MusicPlayer::update() {
  for (auto &update_channel : channels) {
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
}

void MusicPlayer::play(std::uint8_t channel, bool loop, const Note *begin, const Note *end) {
  auto & play_channel = channels[channel];
  play_channel.channel = channel;
  play_channel.begin_note = play_channel.current_note = begin;
  play_channel.end_note = end;
  play_channel.loop = loop;
  play_channel.start_current_note();
}

} // namespace c64
