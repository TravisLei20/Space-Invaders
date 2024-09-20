
#include "Sprite.h"
#include <stdio.h>

// A graphical sprite, which contains a bitmap and width and height. (This only
// tracks the source image, not the GameObject which as an x,y location and
// size)
Sprite::Sprite(const uint32_t data[], uint8_t height, uint8_t width) {
  this->data = new uint32_t[height * width];
  refactor_sprite(data, this->data, height, width);
  this->height = height;
  this->width = width;
}

/*
 * Sprite constructor that will not refactor the data
 */
Sprite::Sprite(const uint32_t data[], uint8_t height, uint8_t width,
               bool no_refactor) {
  this->data = new uint32_t[height * width];
  for (uint32_t i = 0; i < (height * width); i++) {
    this->data[i] = data[i];
  }
  this->height = height;
  this->width = width;
}

Sprite::~Sprite() { delete [] data; }

// Return whether the given row and col idx has a pixel in the sprite.
bool Sprite::isFgPixel(uint8_t row, uint8_t col) {
  printf("Warning: isFgPixel may still be buggy.\n");
  uint16_t location = (row * width) + col;
  return (data[location] == 1);
}

/*
 * Sprite data is given in X bit words. Each bit in each word represents a
 * pixel. This is hard to work with, so refactor_sprite expands the bits
 * into an array i.e. each bit becomes its own value in an array.
 * E.g. 2x8 sprite {0xA8, 0x16} becomes [1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0,
 * 1, 1, 0]
 */
void Sprite::refactor_sprite(const uint32_t *sprite,
                             uint32_t *sprite_refactored, uint8_t num_lines,
                             uint8_t pixels_per_line) {
  // printf("Refactoring Sprite\n");
  int cnt = 0;
  for (uint16_t i = 0; i < num_lines; i++) {
    uint32_t line = sprite[i];
    uint64_t temp = 1 << (pixels_per_line - 1);
    for (uint8_t ii = 0; ii < pixels_per_line; ii++) {
      if (line & temp) {
        // printf("1");
      } else {
        // printf("0");
      }
      sprite_refactored[cnt] = line & temp ? 1 : 0;
      temp = temp >> 1;
      cnt++;
    }
    // printf("\n");
  }
  //   printf("Moved %d pixels into new format\n", cnt);
}