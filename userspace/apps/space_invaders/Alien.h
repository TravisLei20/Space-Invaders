#ifndef ALIEN_H
#define ALIEN_H

#include <stdint.h>

#include "GameObject.h"

class Sprite;

// Class that represents a single alien.
class Alien : public GameObject {
public:
  // Create a new Alien, with given "in" and "out" sprites that are displayed
  // when the alien moves. Make sure this calls the parent constructor.
  Alien(Sprite *spriteIn, Sprite *spriteOut, uint16_t x, uint16_t y);

private:
  Sprite *spriteIn;
  Sprite *spriteOut;
  uint16_t x, y;
  uint8_t size;

  // Whether the 'in' sprite is being displayed
  bool in;

  // Whether the alien is exploding.  When exploding it's not yet dead, but the
  // explosion sprite is displayed.
  bool exploding;
  uint16_t explode_cnt;
  uint16_t total_explode_cnt;

public:
  // Move the alien left, right or down
  void moveLeft();
  void moveRight();
  void moveDown();

  // Trigger the alien to explode
  void explode();
  bool isExploding() { return exploding; }
  void tick();
  Sprite *getStartingSprite() {return spriteIn;}
  void reset();
};

#endif /* ALIEN_H */
