#include "Bullet.h"
#include "GameObject.h"
#include "Globals.h"
#include "Tank.h"
#include "config.h"

#define BULLET_COLOR_AGAIN                                                     \
  { 0xFF, 0xFF, 0xFF }
#define TOP_OF_SCREEN 25
#define BOT_OF_SCREEN 473
#define BULLET_HEIGHT 6

// Use this flag to alternate between the "up" and "down" of each bullet type
// upon drawing/moving
bool alienBulletUp = false;

Bullet::Bullet(Tank *tank)
    : GameObject(Globals::getSprites().getBullet(SPRITE_BULLET_TANK),
                 tank->getMidX(),
                 tank->getY() - (SPRITES_1X5_ROWS * BULLET_SIZE), BULLET_SIZE,
                 BULLET_COLOR_AGAIN) {
  playerBullet = true;
  tickCnt = 0;
  Globals::getAudio().play_audio(Globals::getAudio().LASER, false);
}

// Seg fault here??
// Enemy bullet. Make sure this calls the parent constructor.
Bullet::Bullet(Alien *alien, Sprite *sprite1, Sprite *sprite2)
    : GameObject(this->sprite, alien->getMidX(), alien->getBottomY(),
                 BULLET_SIZE, BULLET_COLOR_AGAIN) {
  playerBullet = false;
  tickCnt = 0;
  this->sprite1 = sprite1;
  this->sprite2 = sprite2;
  this->sprite = sprite1;
  // Globals::getAudio().play_audio(Globals::getAudio().LASER, false);
}

// Seg fault here??
// Enemy bullet. Make sure this calls the parent constructor.
// Bullet::Bullet(Alien *alien, Sprite *sprite1, Sprite *sprite2)
//     : GameObject(sprite1, alien->getMidX(), alien->getY(), BULLET_SIZE,
//                  BULLET_COLOR_AGAIN) {
//   playerBullet = false;
//   tickCnt = 0;
//   this->sprite1 = sprite1;
//   this->sprite2 = sprite2;
// }

bool Bullet::tick() {

  if ((y < TOP_OF_SCREEN) ||
      (y >
       BOT_OF_SCREEN -
           BULLET_HEIGHT)) { // going off the screen. Kill before writing over
                             // stuff (lives, score, etc.)
    kill();
    return false;
  }

  tickCnt++;
  if (tickCnt == tickCntMax) {

    tickCnt = 0;

    if (playerBullet) {
      move(getSprite(), 0, -BULLET_MOVE_Y_DISTANCE_PLAYER);
    }
    // Seg fault here because these bullets don't exist yet??
    else {
      if (alienBulletUp) {
        move(sprite2, 0, BULLET_MOVE_Y_DISTANCE_ENEMY);
        alienBulletUp = false;
      } else {
        move(sprite1, 0, BULLET_MOVE_Y_DISTANCE_ENEMY);
        alienBulletUp = true;
      }
    }
    return true;
  }
  return false;
}

// Kill this bullet (call the parent function kill(), but also notify Bullets
// that the bullet was destroyed)
void Bullet::kill() {
  this->GameObject::kill();
  Globals::getBullets().kill(this);
}