#include "UFO.h"
#include "Colors.h"
#include "GameObject.h"
#include "Globals.h"
#include "config.h"
#include <iostream>
#include <random>

using namespace std;

#define DELAY_MIN                                                              \
  500 // Tick delay for randomizing when the UFO appears, 5 seconds
#define DELAY_MAX                                                              \
  1500 // Tick delay for randomizing when the UFO appears, 15 seconds
#define MOVE_TICKS 5 // Number of ticks before the UFO moves

#define UFO_X_LEFT 10   // Where the UFO will spawn on left
#define UFO_X_RIGHT 607 // Where the UFO will spawn on right

bool move_left_right; // Flag for determining whether the UFO will move left or
                      // right

// UFO Color
Colors ufo_color = Colors();
rgb_t red = ufo_color.RED;

// Constructor
UFO::UFO()
    : GameObject(Globals::getSprites().getUFO(), UFO_X_LEFT, UFO_Y, UFO_SIZE,
                 red) {
  tickCnt = 0;
  hideTickMax = getRandomHideDelayTicks();
  state = HIDDEN;
  move_left_right = false;
}

// Calculate the random number of ticks until the UFO should appear
uint32_t UFO::getRandomHideDelayTicks() {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_int_distribution<uint32_t> dist(DELAY_MIN, DELAY_MAX);
  return dist(gen);
}

bool UFO::tick() {

  // Mealy state machine
  switch (state) {
  case HIDDEN:

    // UFO appears after a random time between 5 and 15 seconds
    if (tickCnt >= hideTickMax) {
      state = MOVING;
      tickCnt = 0;

      Globals::getAudio().play_audio(Globals::getAudio().UFO, true);

      // Randomize whether the UFO starts from the left or right
      if (hideTickMax % 2) {
        move_left_right = true;
        x = UFO_X_LEFT;
      } else {
        move_left_right = false;
        x = UFO_X_RIGHT;
      }
    }

    break;

  case MOVING:

    // Disappear when at the right side of the screen
    if (move_left_right && (x >= UFO_X_RIGHT)) {
      state = HIDDEN;
      erase();
      hideTickMax = getRandomHideDelayTicks();
      Globals::getAudio().audio_set_loop(false);
    }
    // Disappear when at the left side of the screen
    else if (!move_left_right && (x <= UFO_X_LEFT)) {
      state = HIDDEN;
      erase();
      hideTickMax = getRandomHideDelayTicks();
      Globals::getAudio().audio_set_loop(false);
    }

    break;
  }

  // Moore state machine
  switch (state) {
  case HIDDEN:

    tickCnt++;

    break;

  case MOVING:

    tickCnt++;

    // Move right
    if (move_left_right && (tickCnt >= MOVE_TICKS)) {
      move(Globals::getSprites().getUFO(), UFO_MOVE_X_DISTANCE, 0);
      tickCnt = 0;
    }
    // Move left
    else if (!move_left_right && (tickCnt >= MOVE_TICKS)) {
      move(Globals::getSprites().getUFO(), (-UFO_MOVE_X_DISTANCE), 0);
      tickCnt = 0;
    }

    break;
  }

  return true; // TODO what should we do with this?
}

// Check for collisions between the player bullet and the UFO
void UFO::checkCollisions() {
  Bullet *playerBullet = Globals::getBullets().getPlayerBullet();
  if (isOverlapping(playerBullet)) {
    playerBullet->kill();
    
    // make UFO disappear
    state = HIDDEN;
    erase();
    hideTickMax = getRandomHideDelayTicks();

    Globals::getBullets().kill(playerBullet);
    Globals::getScore().hitSpaceship();
    Globals::getAudio().audio_set_loop(false);
    Globals::getAudio().play_audio(Globals::getAudio().UFO_DIE, false);
  }
}