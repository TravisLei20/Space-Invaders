#ifndef BUNKER_H
#define BUNKER_H

#include <stdint.h>
#include <vector>

#include "Bullet.h"
#include "BunkerBlock.h"
#include "GameObject.h"
#include "Graphics.h"

// Class representing one player bunker
// each bunker is 4 blocks across and 3 blocks tall (without lower middle
// blocks)
// Bunker blocks are "mask" that will be printed over the bunker as parts of the
// bunker are hit with bullets.
class Bunker : public GameObject {
public:
  // Create bunker at given x,y location. Make sure this calls the parent
  // constructor.
  Bunker(uint16_t x, uint16_t y);
  ~Bunker();

private:
  uint16_t dummy_cnt = 0; // REMOVE ME LATER (used for testing)

  // A bunker is comprised of several BunkerBlocks
  std::vector<BunkerBlock *> bunkerBlocks;

  void drawBlocks();

public:
  // Check collision between both player and enemy bullets and the bunker.
  bool checkBulletCollision(Bullet *bullet);
  void checkBlocks();
};

#endif /* BUNKER_H */
