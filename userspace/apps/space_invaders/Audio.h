#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>

class Audio {
public:
  // Initialize the audio configuration
  bool audio_init();

  // Set the volume of the audio codec
  void audio_set_volume(bool up);

  // Set the loop mode of the audio codec
  void audio_set_loop(bool loop);

  enum AudioFile {
    INVADER_DIE,
    LASER,
    PLAYER_DIE,
    UFO_DIE,
    UFO,
    WALK1,
    WALK2,
    WALK3,
    WALK4
  } audio_file;

  // Play the audio file
  bool play_audio(AudioFile file, bool loop);

  void audio_close();

private:
  int fd;         // File descriptor for the device file
  int16_t volume; // Volume of the audio codec
  bool looping;   // Loop mode of the audio codec

  void get_all_audio_buffers();

  int32_t *invader_die;
  int32_t *laser;
  int32_t *player_die;
  int32_t *ufo_die;
  int32_t *ufo;
  int32_t *walk1;
  int32_t *walk2;
  int32_t *walk3;
  int32_t *walk4;

  int invader_die_size;
  int laser_size;
  int player_die_size;
  int ufo_die_size;
  int ufo_size;
  int walk1_size;
  int walk2_size;
  int walk3_size;
  int walk4_size;
};

#endif /* AUDIO_H */