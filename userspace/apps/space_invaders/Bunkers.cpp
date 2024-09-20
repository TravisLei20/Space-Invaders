#include "Bunkers.h"
#include "Graphics.h"
#include "config.h"
#include "Globals.h"

#define BUNKER_WIDTH (BUNKER_SIZE * SPRITES_24X18_COLS)
#define GRAPHICS_SECTION (GRAPHICS_WIDTH / 4)

Bunkers::Bunkers() {
  for (uint8_t i = 0; i < 4; i++) {
    Bunker *bunker =
        new Bunker((GRAPHICS_SECTION * (i + 1) - (GRAPHICS_SECTION / 2)) -
                       (BUNKER_WIDTH / 2),
                   BUNKER_Y);
    bunkers.push_back(bunker);
  }
}

Bunkers::~Bunkers() {
  for (auto bunker : bunkers) {
    // delete bunker;
  }
}

void Bunkers::checkCollisions() {
  for (auto bunker : bunkers) {
    std::vector<Bullet *> enemyBullets = Globals::getBullets().getEnemyBullets();
    for (auto bullet: enemyBullets) {
      bunker->checkBulletCollision(bullet);
    }
    if (Globals::getBullets().getPlayerBullet()) {
      bunker->checkBulletCollision(Globals::getBullets().getPlayerBullet());
    }
    bunker->checkBlocks();
  }
}