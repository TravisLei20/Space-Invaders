#include <cstdint>
#include <cstdio>
#include <string>

#include "Graphics.h"
#include "Sprites.h"
#include "Globals.h"

Graphics::Graphics() {
  fd = open(SYSTEM_HDMI_FILE, O_RDWR);
  // Check that UIO file opened correctly.
  if (fd < 0) {
    printf("Cannot open %s. Did you sudo?\n", SYSTEM_HDMI_FILE);
    exit(1);
  }
}
Graphics::Graphics(int fd) { this->fd = fd; };

// move cursor to desired x and y position (given in pixels)
void Graphics::move_cursor(uint16_t x, uint16_t y) {
  lseek(fd, (x + (y * GRAPHICS_WIDTH)) * PIXEL_MULTIPLIER, SEEK_SET);
}

// fills a buffer with the color for each pixel
void Graphics::fill_buffer(uint8_t *buf, rgb_t color, uint32_t num_pixels) {
  int count = 0;
  for (uint32_t i = 0; i < (num_pixels * PIXEL_MULTIPLIER);) {
    buf[i++] = color.r;
    buf[i++] = color.g;
    buf[i++] = color.b;
  }
}

/*
 * Fill the screen with the given color
 */
void Graphics::fillScreen(rgb_t color) {

  // array that contains all values needed to fill the screen
  uint8_t screen[GRAPHICS_HEIGHT * GRAPHICS_WIDTH * PIXEL_MULTIPLIER];

  fill_buffer(screen, color, GRAPHICS_HEIGHT * GRAPHICS_WIDTH);

  move_cursor(0, 0);

  write(fd, screen, (GRAPHICS_HEIGHT * GRAPHICS_WIDTH * PIXEL_MULTIPLIER));
  // printf("Done filling screen\n");
}

void Graphics::drawGround() {

  rgb_t color = {0x00, 0xFF, 0x00};
  uint8_t num_rows = 1;

  uint8_t data[GRAPHICS_WIDTH * PIXEL_MULTIPLIER * num_rows];

  fill_buffer(data, color, GRAPHICS_WIDTH * num_rows);

  move_cursor(0, GRAPHICS_HEIGHT - num_rows);

  write(fd, data, (GRAPHICS_WIDTH * PIXEL_MULTIPLIER * num_rows));
  // printf("Done drawing ground\n");
}

// This draws a sprite of given size and color at an x,y location.  This
// version of the function is given a background color (bgColor), such that
// every pixel of sprite region is written (with either color or bgColor).
// This is faster because it allows you to write line by line and minimize
// system calls.
void Graphics::drawSprite(Sprite *sprite, uint16_t x, uint16_t y, uint8_t size,
                          rgb_t color, rgb_t bgColor) {

  Sprite *sprite_to_print;

  // scale the sprite if necessary
  if (size == 1) {
    sprite_to_print = sprite;
  } else {
    sprite_to_print = scaleSprite(sprite, size);
  }

  // get Sprite pixel data
  uint32_t *sprite_data = sprite_to_print->getData();

  // printf("Printing sprite to screen\n");

  int cnt = 0;

  // for each line
  for (uint16_t i = 0; i < sprite_to_print->getHeight(); i++) {

    // navigate to start of line
    move_cursor(x, y + i);

    uint8_t line[sprite_to_print->getWidth() * PIXEL_MULTIPLIER];
    int line_cnt = 0;

    // get pixel data in each line
    for (uint8_t ii = 0; ii < sprite_to_print->getWidth(); ii++) {
      if (sprite_data[cnt]) {
        line[line_cnt++] = color.r;
        line[line_cnt++] = color.g;
        line[line_cnt++] = color.b;
      } else {
        line[line_cnt++] = bgColor.r;
        line[line_cnt++] = bgColor.g;
        line[line_cnt++] = bgColor.b;
      }
      //   printf("%d", sprite_data[cnt]);
      cnt++;
    }
    // printf("\n");
    // draw the line all at once
    write(fd, line, sprite_to_print->getWidth() * PIXEL_MULTIPLIER);
  }
  // printf("Printed %d pixels\n", cnt);

  // delete scaled sprite
  if (size != 1) {
    delete sprite_to_print;
  }
}

// Same as previous function, but does not write over the background pixels.
// Although this writes fewer pixels, it often requires more system calls and
// so will be slower.  This is needed to draw the bunker damage.
void Graphics::drawSprite(Sprite *sprite, uint16_t x, uint16_t y, uint8_t size,
                          rgb_t color) {

  Sprite *sprite_to_print;

  // scale the sprite if necessary
  if (size == 1) {
    sprite_to_print = sprite;
  } else {
    sprite_to_print = scaleSprite(sprite, size);
  }

  // get Sprite pixel data
  uint32_t *sprite_data = sprite_to_print->getData();

  // printf("Printing sprite to screen\n");

  int cnt = 0;

  // for each line
  for (uint16_t i = 0; i < sprite_to_print->getHeight(); i++) {

    // navigate to start of line
    move_cursor(x, y + i);

    // draw each pixel in the line
    for (uint8_t ii = 0; ii < sprite_to_print->getWidth(); ii++) {
      if (sprite_data[cnt]) {
        write(fd, &color, PIXEL_MULTIPLIER);
      } else {
        lseek(fd, PIXEL_MULTIPLIER, SEEK_CUR);
      }
      //   printf("%d", sprite_data[cnt]);
      cnt++;
    }
    // printf("\n");
  }
  // printf("Printed %d pixels\n", cnt);

  // delete scaled sprite
  if (size != 1) {
    delete sprite_to_print;
  }
}

/*
 * Draws a string on the screen, and returns the width
 */
uint16_t Graphics::drawStr(std::string str, uint16_t x, uint16_t y,
                           uint8_t size, rgb_t color) {

  uint16_t x_current = x;
  uint16_t y_current = y;
  uint16_t space = size;

  // draw each character
  for (char &c : str) {

    Sprite *letter = Globals::getSprites().getChar(c);
    drawSprite(letter, x_current, y_current, size, color);

    // update x position
    x_current += SPRITES_5X5_COLS * size;
    if (c != ' ') {
      x_current += space;
    }
  }
  return x_current - x;
}

/*
 * Draws a string on the screen that is centered horizontally
 */
void Graphics::drawStrCentered(std::string str, uint16_t y, uint8_t size,
                               rgb_t color) {
  // get width of the string
  uint16_t width = getStrWidth(str.size(), size);

  // determine starting x position
  uint16_t x = (GRAPHICS_WIDTH - width) / 2;

  // draw the string
  drawStr(str, x, y, size, color);
}

// Returns the width of a str of given length and size.  This should take into
// account the character sizes, and spacing between characters.  For a given
// string and size, this should return the same value as drawStr.
uint16_t Graphics::getStrWidth(uint8_t strLen, uint8_t size) {
  uint16_t spaces = (strLen - 1) * size;
  uint16_t charSize = SPRITES_5X5_COLS * size;
  return strLen * charSize + spaces;
}

/*
 * Returns a Sprite object scaled by scale_factor
 */
Sprite *Graphics::scaleSprite(Sprite *sprite, uint8_t scale_factor) {

  // create a new array of appropriate size (height * scale)(width * scale)
  uint32_t data_scaled[sprite->getHeight() * sprite->getWidth() * scale_factor *
                       scale_factor];

  // get unscaled data
  uint32_t *sprite_data = sprite->getData();

  int cnt = 0;

  // for each line in the original data
  for (uint8_t line_num = 0; line_num < sprite->getHeight(); line_num++) {

    // repeat scale_factor times
    for (uint8_t scale_cnt = 0; scale_cnt < scale_factor; scale_cnt++) {

      // for each pixel in the line
      for (uint8_t i = 0; i < sprite->getWidth(); i++) {
        uint8_t pixel_value = sprite_data[line_num * sprite->getWidth() + i];

        // repeat scale_factor times
        for (uint8_t ii = 0; ii < scale_factor; ii++) {
          data_scaled[cnt] = pixel_value;
          cnt++;
          //   printf("%d", pixel_value);
        }
      }
      //   printf("\n");
    }
  }
  // printf("Scaled to %d pixels\n", cnt);

  // put scaled data into new sprite object
  Sprite *scaledSprite =
      new Sprite(data_scaled, sprite->getHeight() * scale_factor,
                 sprite->getWidth() * scale_factor, false);
  return scaledSprite;
}
