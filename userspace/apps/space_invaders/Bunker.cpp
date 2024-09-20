#include <stdint.h>
#include <vector>

#include "Bunker.h"
#include "BunkerBlock.h"
#include "GameObject.h"
#include "Globals.h"
#include "Graphics.h"
#include "Sprite.h"
#include "Sprites.h"
#include "config.h"

#define BUNKER_COLOR_AGAIN                                                     \
  { 0x00, 0xFF, 0x00 }
#define BLOCK_WIDTH SPRITES_6X6_COLS *BUNKER_SIZE
#define BLOCK_HEIGHT SPRITES_6X6_ROWS *BUNKER_SIZE

// Create bunker at given x,y location. Make sure this calls the parent
// constructor.
Bunker::Bunker(uint16_t x, uint16_t y)
    : GameObject(Globals::getSprites().getBunker(), x, y, BUNKER_SIZE,
                 BUNKER_COLOR_AGAIN) {
  draw();

  // printf("Bunker is at x: %d y: %d\n", x, y);

  // each bunker is 4 blocks across and 3 blocks tall (without lower middle
  // blocks)
  for (uint8_t i = 0; i < 3; i++) {
    for (uint8_t j = 0; j < 4; j++) {
      if ((i == 2) && ((j == 1) || (j == 2))) { // skip lower middle blocks
        continue;
      }

      // printf("\tNew block at x: %d y: %d\n", (x + (BLOCK_WIDTH * j)),
      //  (y + BLOCK_HEIGHT * i));

      BunkerBlock *block =
          new BunkerBlock(x + (BLOCK_WIDTH * j), y + (BLOCK_HEIGHT * i));
      bunkerBlocks.push_back(block);
    }
  }
  drawBlocks();
}

Bunker::~Bunker() {
  for (auto block : bunkerBlocks) {
    delete block;
  }
}

void Bunker::drawBlocks() {
  for (auto block : bunkerBlocks) {
    block->drawNoBackground();
  }
}

void Bunker::checkBlocks() {
  std::vector<BunkerBlock *> updatedBlocks;
  for (auto block: bunkerBlocks) {
    if (block->getDmgLevel() == BUNKER_BLOCK_MAX_DAMAGE) {
      block->kill();
    }
    else {
      updatedBlocks.push_back(block);
    }
  }
  bunkerBlocks = updatedBlocks;
}

bool between(uint32_t spot, uint32_t bound1, uint32_t bound2) {
  return (spot > bound1) && (spot < bound2);
}

// Check collision between both player and enemy bullets and the bunker.
bool Bunker::checkBulletCollision(Bullet *bullet) {
  if (bullet->isAlive()) {
    for (auto block : bunkerBlocks) {
      if (block->isAlive() && block->isOverlapping(bullet)) {
        block->inflictDamage();
        block->drawNoBackground();
        bullet->kill();
        return true;
      }
    }
    return false;
  } else {
    return false;
  }
}

////////////////// Erodes bunkers every 200 interrupts (for testing)
///////////////////////
// dummy_cnt += 1;
// bool do_damage = false;
// if (dummy_cnt == 200) {
//   do_damage = true;
//   dummy_cnt = 0;
// }

// for (auto block : bunkerBlocks) {
//   if (block->getDmgLevel() == 4) {
//     continue;
//   }
//   if (do_damage) {
//     block->inflictDamage();
//   }
// }
// drawBlocks();
////////////////////////////////////////////////////////////
