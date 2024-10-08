#ifndef TANK_H
#define TANK_H

#include <stdint.h>

#include "Colors.h"
#include "GameObject.h"
#include "Sprites.h"

// The player tank (can also be used to draw the tanks for the lives)
class Tank : public GameObject {
public:
  // Player tank (initialized to starting locating)
  Tank();

  // Tank for player lives, drawn at given x,y coordinate
  Tank(uint16_t x, uint16_t y);

private:
  // poll values of buttons
  void check_buttons();

  // Tick counter and max tick for moving the tank
  uint32_t tickCnt;
  const uint32_t tickMax = 10;      // TODO fix this if needed
  const uint32_t deathTickMax = 10; // TODO fix this if needed

  // Tank state, which is either alive or switching between the two explosion
  // sprites for a given time.
  typedef enum {
    TANK_STATE_ALIVE,
    TANK_STATE_DYING1,
    TANK_STATE_DYING2
  } tank_state_t;
  tank_state_t state;

  // Used to flag the state machine that the tank should be destroyed.
  bool flagExplosion;

  uint8_t buttons_raw = 0;
  uint8_t buttons = 0;
  uint8_t debounce_counter = 0;
  uint8_t move_cnt = 0;
  uint32_t death_cnt = 0;
  uint16_t explosion_cnt = 0;
  bool usingExplosion1 = true;

public:
  // Tick the tank
  bool tick();

  // Check for collisions between alien bullets and the tank
  void checkCollisions();

  // Kill the tank.  This should flag the explosion (but don't call
  // GameObject.kill(), otherwise the tank won't be drawn)
  void kill();

  void btn_interrupt();
};

#endif /* TANK_H */
