#ifndef BULLET_H
#define BULLET_H

#include <stdint.h>

#include "Alien.h"
#include "GameObject.h"
#include "Tank.h"
#include "config.h"

// Class to represent player and enemy bullets
class Bullet : public GameObject {
public:
  // Player bullet. Make sure this calls the parent constructor.
  Bullet(Tank *tank);

  // Enemy bullet. Make sure this calls the parent constructor.
  Bullet(Alien *alien, Sprite *sprite1, Sprite *sprite2);

private:
  // Whether bullet is a player bullet
  bool playerBullet;

  // Bullets alternate between two sprites for the wiggle effect.
  // These store the "up" and "down" versions of the given bullet type.
  Sprite *sprite1;
  Sprite *sprite2;

  // Tick counter and max count for moving bullets
  uint32_t tickCnt;
  uint32_t tickCntMax = BULLET_MOVE_DELAY_SECONDS * 100;

public:
  // Tick the bullet, and return whether it moved.
  bool tick();

  // Kill this bullet (call the parent function kill(), but also notify Bullets
  // that the bullet was destroyed)
  void kill();
};

#endif /* BULLET_H */
