#include <fcntl.h>
#include <linux/i2c.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <sys/ioctl.h>

#include "./../../drivers/audio_config/audio_config.h"

#define DEVICE_PATH "/dev/audio_codec"

#define VOLUME 50
#define HEADER_SIZE 44

#define AUDIO_IOCTL_SET_LOOP _IO('A', 0)
#define AUDIO_IOCTL_CLEAR_LOOP _IO('A', 1)

int main() {

  audio_config_init();
  audio_config_set_volume(VOLUME);

  const char *wave_file_path = "../../resources/wavFiles/ufo.wav";
  std::ifstream wave_file(wave_file_path, std::ios::binary);
  if (!wave_file.is_open()) {
      printf("Failed to open file: %s\n", wave_file_path);
      return 1;
  }

  // Skip past the header (assuming a 44-byte header for simplicity)
  wave_file.seekg(0, std::ios::end);

  int file_size = wave_file.tellg();
  file_size -= HEADER_SIZE; // Subtract the header size

  wave_file.seekg(HEADER_SIZE, std::ios::beg);

  int16_t *buffer = new int16_t[file_size/2];           // Assuming 16-bit samples
  int32_t *converted_buffer = new int32_t[file_size/2]; // 32-bit samples

  int fd; // File descriptor for the device file

  // Open the device file
  fd = open(DEVICE_PATH, O_RDWR);
  if (fd < 0) {
    perror("Failed to open the device file");
    return EXIT_FAILURE;
  }

  wave_file.read(reinterpret_cast<char*>(buffer), file_size);

  for (size_t i = 0; i < file_size/2; i++) {
    // Convert 16-bit sample to 32-bit (24-bit PCM)
    converted_buffer[i] = (int32_t)buffer[i] << 8;
  }

  if (ioctl(fd, AUDIO_IOCTL_SET_LOOP) < 0) {
      perror("Failed to set loop");
      close(fd);
      return 1;
  }

  if (write(fd, converted_buffer, file_size*2) != file_size*2) {
    perror("Failed to write to the device file\n");
    return EXIT_FAILURE;
  }

  // Prompt the user for input
  std::cout << "Enter a number and press enter to end loop: ";

  // Get input from the user
  int number;
  std::cin >> number;

  // Clear looping
  if (ioctl(fd, AUDIO_IOCTL_CLEAR_LOOP) < 0) {
      perror("Failed to clear loop");
      close(fd);
      return 1;
  }

  // Close the device file
  close(fd);
  // Close the wave file
  wave_file.close();

  return 0;
}
