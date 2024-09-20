#include "Tank.h"
#include "Bullets.h"
#include "Colors.h"
#include "GameObject.h"
#include "Globals.h"
#include "Sprites.h"
#include "config.h"
#include "drivers/buttons/buttons.h"
#include "drivers/switches/switches.h"
#include <iostream>

using namespace std;

#define TANK_START_X 10
#define TANK_COLOR_AGAIN                                                       \
  { 0, 0xFF, 0 }
#define DEBOUNCE_MAX 5
#define MOVE_CNT_MAX 1

bool volume_changed = false;

const int EXIT_ERROR = -1;

Tank::Tank()
    : GameObject(Globals::getSprites().getTank(SPRITE_TANK), TANK_START_X,
                 TANK_Y, TANK_SIZE, TANK_COLOR_AGAIN) {
  tickCnt = 0;
  flagExplosion = false;
  draw();
}

// Tank for player lives, drawn at given x,y coordinate
Tank::Tank(uint16_t x, uint16_t y)
    : GameObject(Globals::getSprites().getTank(SPRITE_TANK), x, y, TANK_SIZE,
                 TANK_COLOR_AGAIN) {
  tickCnt = 0;
  flagExplosion = false;
  draw();
}

// Tick the tank
bool Tank::tick() {
  if (flagExplosion) {
    death_cnt++;
    explosion_cnt++;
    if (death_cnt == (TANK_DEATH_DURATION_SECONDS * 100)) {
      flagExplosion = false;
      death_cnt = 0;
      explosion_cnt = 0;
      move(Globals::getSprites().getTank(SPRITE_TANK), -(x - TANK_START_X), 0);
      usingExplosion1 = false;
    } else if (explosion_cnt >= deathTickMax) {
      // change sprite
      if (usingExplosion1) {
        move(Globals::getSprites().getTank(SPRITE_TANK_EXPLOSION1), 0, 0);
        usingExplosion1 = false;
      } else {
        move(Globals::getSprites().getTank(SPRITE_TANK_EXPLOSION2), 0, 0);
        usingExplosion1 = true;
      }
      return false; // do nothing else because tank is currently exploding
    }
  }
  if (debounce_counter < DEBOUNCE_MAX) {
    debounce_counter++;
    move_cnt = 0;
  } else {
    buttons = buttons_raw;
  }

  if (buttons) {
    // change volume
    if (buttons & BUTTONS_3_MASK && !volume_changed) {
      uint32_t switches = switches_read();
      if (switches & SWITCHES_0_MASK) {
        Globals::getAudio().audio_set_volume(true);
      } else {
        Globals::getAudio().audio_set_volume(false);
      }
      volume_changed = true;
    }

    // fire bullet
    if (buttons & BUTTONS_1_MASK) {
      if (Globals::getBullets().getPlayerBullet() == nullptr) {
        Globals::getBullets().newPlayerBullet(this);
      }
    }

    // move tank
    if (move_cnt < MOVE_CNT_MAX) {
      move_cnt++;
    } else {
      move_cnt = 0;

      uint16_t x_new = x;

      // move tank right
      if (buttons & BUTTONS_0_MASK) {
        if (x < (GRAPHICS_WIDTH - TANK_WIDTH)) {
          x_new += TANK_MOVE_X_DISTANCE;
        } else {
          x_new = GRAPHICS_WIDTH - TANK_WIDTH;
        }
      }

      // move tank left
      if (buttons & BUTTONS_2_MASK) {
        if ((x > TANK_WIDTH) || (x > TANK_MOVE_X_DISTANCE)) {
          x_new -= TANK_MOVE_X_DISTANCE;
        } else {
          x_new = 0;
        }
      }

      // redraw tank in new position
      if (x != x_new) {
        erase();
        x = x_new;
        draw();
      }
    }
  }

  return false; // TODO CHANGE THIS
}

void Tank::btn_interrupt() {
  buttons_raw = buttons_read();
  if (buttons_raw == 0) {
    volume_changed = false;
  }
  debounce_counter = 0;
}

// Check for collisions between alien bullets and the tank
void Tank::checkCollisions() {
  if (flagExplosion) {
    return;
  }

  std::vector<Bullet *> enemyBullets = Globals::getBullets().getEnemyBullets();
  for (auto bullet : enemyBullets) {
    if (isOverlapping(bullet)) {
      kill();
      bullet->kill();
    }
  }
}

// Kill the tank.  This should flag the explosion (but don't call
// GameObject.kill(), otherwise the tank won't be drawn)
void Tank::kill() {
  flagExplosion = true;
  Globals::getLives().loseALife();
  Globals::getAudio().play_audio(Globals::getAudio().PLAYER_DIE, false);
}