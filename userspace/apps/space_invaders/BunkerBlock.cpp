#include "BunkerBlock.h"
#include "GameObject.h"
#include "Globals.h"
#include "Sprite.h"
#include "Sprites.h"
#include "config.h"

#define BUNKERBLOCK_COLOR                                                      \
  { 0x00, 0x00, 0x00 }

BunkerBlock::BunkerBlock(uint16_t x, uint16_t y)
    : GameObject(Globals::getSprites().getBunkerDmg(0), x, y, BUNKER_SIZE,
                 BUNKERBLOCK_COLOR) {
  dmgLevel = 0;
}

BunkerBlock::~BunkerBlock() {}

void BunkerBlock::inflictDamage() {
  dmgLevel++;
  sprite = Globals::getSprites().getBunkerDmg(dmgLevel);
}
