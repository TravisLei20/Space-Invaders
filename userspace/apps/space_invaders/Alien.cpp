#include "Alien.h"
#include "Audio.h"
#include "Colors.h"
#include "GameObject.h"
#include "Globals.h"
#include "Sprites.h"
#include "config.h"

// Alien Color
Colors alien_color = Colors();
rgb_t white = alien_color.WHITE;
#define ALIEN_EXPLOSION_TIME 20

// Constructor
Alien::Alien(Sprite *spriteIn, Sprite *spriteOut, uint16_t x, uint16_t y)
    : GameObject(this->sprite, x, y, ALIENS_SIZE, white) {
  this->spriteIn = spriteIn;
  this->spriteOut = spriteOut;
  this->sprite = spriteIn;
  this->exploding = false;
  this->in = true;
  explode_cnt = 0;
  draw();
}

void Alien::moveLeft() {
  if (exploding) {
    return;
  }
  if (in) {
    move(spriteOut, (-ALIENS_MOVE_X_DISTANCE), 0);
    in = false;
  } else {
    move(spriteIn, (-ALIENS_MOVE_X_DISTANCE), 0);
    in = true;
  }
}

void Alien::moveDown() {
  if (exploding) {
    return;
  }
  if (in) {
    move(spriteOut, 0, ALIENS_MOVE_Y_DISTANCE);
    in = false;
  } else {
    move(spriteIn, 0, ALIENS_MOVE_Y_DISTANCE);
    in = true;
  }
}

void Alien::moveRight() {
  if (exploding) {
    return;
  }
  if (in) {
    move(spriteOut, (ALIENS_MOVE_X_DISTANCE), 0);
    in = false;
  } else {
    move(spriteIn, (ALIENS_MOVE_X_DISTANCE), 0);
    in = true;
  }
}

void Alien::explode() {
  exploding = true;
  move(Globals::getSprites().getAlienExplosion(), 0, 0);
  Globals::getAudio().play_audio(Globals::getAudio().INVADER_DIE, false);
}

void Alien::tick() {
  if (exploding) {
    explode_cnt++;
    if (explode_cnt == ALIEN_EXPLOSION_TIME) {
      kill();
      explode_cnt = 0;
      exploding = false;
    }
  }
}

void Alien::reset() {
  exploding = false;
  explode_cnt = 0;
  exploding = false;
  in = true;
  sprite = spriteIn;
}