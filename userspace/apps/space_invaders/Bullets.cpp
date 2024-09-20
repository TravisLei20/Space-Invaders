#include "Bullets.h"
#include "Globals.h"
#include <algorithm>
#include <cstdlib>

Bullets::Bullets() {}

Bullets::~Bullets() {
  if (playerBullet != nullptr) {
    delete playerBullet;
  }
  for (auto &bullet : enemyBullets) {
    delete bullet;
  }
}

void Bullets::newPlayerBullet(Tank *tank) { playerBullet = new Bullet(tank); }

void Bullets::newEnemyBullet(Alien *alien) {

  // Randomly select which type of bullet the alien will fire.
  sprite_bullet_type_t sprite1Index;
  sprite_bullet_type_t sprite2Index;

  if (rand() % 2) {
    sprite1Index = SPRITE_BULLET_ALIEN1_DOWN;
    sprite2Index = SPRITE_BULLET_ALIEN1_UP;
  } else {
    sprite1Index = SPRITE_BULLET_ALIEN2_DOWN;
    sprite2Index = SPRITE_BULLET_ALIEN2_UP;
  }

  Bullet *bullet =
      new Bullet(alien, Globals::getSprites().getBullet(sprite1Index),
                 Globals::getSprites().getBullet(sprite2Index));
  enemyBullets.push_back(bullet);

  // Testing to see where the seg fault is coming from
  // Bullet *bullet = new Bullet(
  //     alien, Globals::getSprites().getBullet(SPRITE_BULLET_ALIEN1_DOWN),
  //     Globals::getSprites().getBullet(SPRITE_BULLET_ALIEN1_UP));
  // enemyBullets.push_back(bullet);
}

bool Bullets::tick() {
  bool moved = false;
  if (playerBullet != nullptr) {
    if (playerBullet->tick()) {
      moved = true;
    }
  }
  for (auto bullet : enemyBullets) {
    if (bullet->tick()) {
      moved = true;
    }
  }
  return moved;
}

// Kill this bullet (deallocate and remove, updating playerBullet or
// enemyBullets)
void Bullets::kill(Bullet *bullet) {
  if (bullet == playerBullet) {
    delete bullet;
    playerBullet = nullptr;
    
  } 
  else {
    auto it = std::find(enemyBullets.begin(), enemyBullets.end(), bullet);
    if (it != enemyBullets.end()) {
      delete *it;
      enemyBullets.erase(it);
    }
  }
  
}