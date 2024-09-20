#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "system.h"
#include <cstdint>
#include <cstdio>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/mman.h>
#include <unistd.h>

#include "Sprite.h"

#define GRAPHICS_WIDTH 640
#define GRAPHICS_HEIGHT 480
#define PIXEL_MULTIPLIER 3

// An RGB pixel/color
typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} rgb_t;

// Used to draw graphics to HDMI pixel buffer
class Graphics {
public:
  Graphics();
  Graphics(int fd);

private:
  // File descriptor for opened HDMI device file
  int fd;

  // move cursor to desired x and y position (given in pixels)
  void move_cursor(uint16_t x, uint16_t y);

  // fills a buffer with the color for each pixel
  void fill_buffer(uint8_t *buf, rgb_t color, uint32_t num_pixels);

public:
  // Fill the screen.  This is fastest if you write line by line.
  void fillScreen(rgb_t color);

  // draw the green ground
  void drawGround();

  // This draws a sprite of given size and color at an x,y location.  This
  // version of the function is given a background color (bgColor), such that
  // every pixel of sprite region is written (with either color or bgColor).
  // This is faster because it allows you to write line by line and minimize
  // system calls.
  void drawSprite(Sprite *sprite, uint16_t x, uint16_t y, uint8_t size,
                  rgb_t color, rgb_t bgColor);

  // Same as previous function, but does not write over the background pixels.
  // Although this writes fewer pixels, it often requires more system calls and
  // so will be slower.  This is needed to draw the bunker damage.
  void drawSprite(Sprite *sprite, uint16_t x, uint16_t y, uint8_t size,
                  rgb_t color);

  // Draws a string on the screen, and returns the width
  uint16_t drawStr(std::string str, uint16_t x, uint16_t y, uint8_t size,
                   rgb_t color);

  // Draws a string on the screen that is centered horizontally
  void drawStrCentered(std::string str, uint16_t y, uint8_t size, rgb_t color);

  // Returns the width of a str of given length and size.  This should take into
  // account the character sizes, and spacing between characters.  For a given
  // string and size, this should return the same value as drawStr.
  uint16_t getStrWidth(uint8_t strLen, uint8_t size);

  Sprite *scaleSprite(Sprite *sprite, uint8_t scale_factor);
};

#endif /* GRAPHICS_H */
