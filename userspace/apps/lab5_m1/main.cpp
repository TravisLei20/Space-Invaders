#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define DEVICE_PATH "/dev/audio_codec"

#define BUFFER_SIZE 1000

int main() {
  int fd; // File descriptor for the device file
  char buffer[BUFFER_SIZE];

  // Open the device file
  fd = open(DEVICE_PATH, O_RDWR);
  if (fd < 0) {
    perror("Failed to open the device file");
    return EXIT_FAILURE;
  }

  read(fd, buffer, BUFFER_SIZE);

  write(fd, buffer, BUFFER_SIZE);

  // Close the device file
  close(fd);

  return EXIT_SUCCESS;
}
