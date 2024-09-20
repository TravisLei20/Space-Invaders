#include "Audio.h"
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <linux/i2c.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "./../../drivers/audio_config/audio_config.h"

using namespace std;

#define DEVICE_PATH "/dev/audio_codec"

#define DEFAULT_VOLUME 39
#define CHANGE_VOLUME_STEP 6
#define HEADER_SIZE 44

#define AUDIO_IOCTL_SET_LOOP _IO('A', 0)
#define AUDIO_IOCTL_CLEAR_LOOP _IO('A', 1)

bool Audio::audio_init() {
  // Implementation of audio configuration initialization
  std::cout << "Initializing audio configuration..." << std::endl;
  audio_config_init();
  volume = DEFAULT_VOLUME;
  audio_set_volume(volume);

  get_all_audio_buffers();

  looping = false;

  // Open the device file
  fd = open(DEVICE_PATH, O_RDWR);
  if (fd < 0) {
    perror("Failed to open the device file");
    return false;
  }
  return true;
}

void Audio::audio_set_volume(bool up) {
  if (up) {
    volume += CHANGE_VOLUME_STEP;
  } else {
    volume -= CHANGE_VOLUME_STEP;
  }

  if (volume > 63) {
    volume = 63;
  } else if (volume < 0) {
    volume = 0;
  }

  std::cout << "Setting audio volume to " << static_cast<int>(volume)
            << std::endl;

  audio_config_set_volume((int8_t)volume);
}

void Audio::audio_set_loop(bool loop) {
  // Implementation of setting the loop mode of the audio codec
  if (loop) {
    std::cout << "Enabling audio looping" << std::endl;
    if (ioctl(fd, AUDIO_IOCTL_SET_LOOP) < 0) {
      perror("Failed to set loop");
      close(fd);
    }
  } else {
    std::cout << "Disabling audio looping" << std::endl;
    if (ioctl(fd, AUDIO_IOCTL_CLEAR_LOOP) < 0) {
      perror("Failed to clear loop");
      close(fd);
    }
  }
  looping = loop;
}

bool Audio::play_audio(AudioFile file, bool loop) {
  if (volume == 0) {
    if (looping) {
      audio_set_loop(false);
      looping = false;
    }
    return true;
  }

  if (looping) {
    return true;
  }

  if (loop) {
    audio_set_loop(true);
    looping = true;
  }

  int32_t *buffer;
  int file_size;

  switch (file) {
  case INVADER_DIE:
    buffer = invader_die;
    file_size = invader_die_size;
    break;
  case LASER:
    buffer = laser;
    file_size = laser_size;
    break;
  case PLAYER_DIE:
    buffer = player_die;
    file_size = player_die_size;
    break;
  case UFO_DIE:
    buffer = ufo_die;
    file_size = ufo_die_size;
    break;
  case UFO:
    buffer = ufo;
    file_size = ufo_size;
    break;
  case WALK1:
    buffer = walk1;
    file_size = walk1_size;
    break;
  case WALK2:
    buffer = walk2;
    file_size = walk2_size;
    break;
  case WALK3:
    buffer = walk3;
    file_size = walk3_size;
    break;
  case WALK4:
    buffer = walk4;
    file_size = walk4_size;
    break;
  default:
    return false;
  }

  if (write(fd, buffer, file_size) != file_size) {
    perror("Failed to write to the device file\n");
    return false;
  }

  return true;
}

void Audio::audio_close() {
  // Implementation of closing the audio configuration
  std::cout << "Closing audio configuration..." << std::endl;
  audio_set_loop(false);
  close(fd);
}

bool get_audio_buffer(int32_t *&buf, int &size, const char *wave_file_path) {
  std::cout << "Getting audio buffer... " << wave_file_path << std::endl;
  std::ifstream wave_file(wave_file_path, std::ios::binary);
  if (!wave_file.is_open()) {
    printf("Failed to open file: %s\n", wave_file_path);
    return false;
  }
  // Skip past the header (assuming a 44-byte header for simplicity)
  wave_file.seekg(0, std::ios::end);

  int file_size = wave_file.tellg();
  file_size -= HEADER_SIZE; // Subtract the header size

  wave_file.seekg(HEADER_SIZE, std::ios::beg);

  int16_t *buffer = new int16_t[file_size / 2]; // Assuming 16-bit samples
  buf = new int32_t[file_size / 2];             // 32-bit samples

  wave_file.read(reinterpret_cast<char *>(buffer), file_size);

  // Close the wave file
  wave_file.close();

  for (size_t i = 0; i < file_size / 2; i++) {
    // Convert 16-bit sample to 32-bit (24-bit PCM)
    buf[i] = (int32_t)buffer[i] << 8;
  }

  size = file_size * 2;

  return true;
}

void Audio::get_all_audio_buffers() {
  // Implementation of getting all audio buffers
  std::cout << "Getting all audio buffers..." << std::endl;
  get_audio_buffer(invader_die, invader_die_size,
                   "../../resources/wavFiles/invader_die.wav");
  get_audio_buffer(laser, laser_size, "../../resources/wavFiles/laser.wav");
  get_audio_buffer(player_die, player_die_size,
                   "../../resources/wavFiles/player_die.wav");
  get_audio_buffer(ufo_die, ufo_die_size,
                   "../../resources/wavFiles/ufo_die.wav");
  get_audio_buffer(ufo, ufo_size, "../../resources/wavFiles/ufo.wav");
  get_audio_buffer(walk1, walk1_size, "../../resources/wavFiles/walk1.wav");
  get_audio_buffer(walk2, walk2_size, "../../resources/wavFiles/walk2.wav");
  get_audio_buffer(walk3, walk3_size, "../../resources/wavFiles/walk3.wav");
  get_audio_buffer(walk4, walk4_size, "../../resources/wavFiles/walk4.wav");
}
