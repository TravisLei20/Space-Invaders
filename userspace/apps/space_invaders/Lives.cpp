#include "Lives.h"
#include "Globals.h"
#include "Tank.h"
#include "config.h"

#define LIVES_TEXT_WIDTH SPRITES_5X5_COLS
#define LIVES_TEXT_HEIGHT SPRITES_5X5_ROWS
#define LIVES_NUM_CHARS 5
#define LIVES_SPACE_USED                                                       \
  LIVES_NUM_CHARS *LIVES_TEXT_WIDTH *LIVES_TEXT_SIZE + LIVES_NUM_CHARS * 2
#define TANK_LIVES_STARTING_X LIVES_X + LIVES_SPACE_USED + LIVES_X_PAD
#define WHITE                                                                  \
  { 0xFF, 0xFF, 0xFF }
#define LIVES_ICON_Y (LIVES_Y - (TANK_HEIGHT - (LIVES_TEXT_HEIGHT * LIVES_TEXT_SIZE)))

Lives::Lives() {
  numLives = LIVES_AT_START;
  xDrawTankStart = TANK_LIVES_STARTING_X;
  for (uint8_t i = 0; i < LIVES_MAX; i++) {
    Tank *tank = new Tank(xDrawTankStart, LIVES_ICON_Y);
    tankLives.push_back(tank);
    xDrawTankStart += TANK_WIDTH + LIVES_X_PAD;
  }
  numLives = 3;
  draw();
}

// Draw the lives at the start of the game
void Lives::draw() {
  Globals::getGraphics().drawStr("LIVES", LIVES_X, LIVES_Y, LIVES_TEXT_SIZE,
                                 WHITE);
  
  int cnt = 0;
  for (auto life : tankLives) {
    if (cnt == numLives) {
      break;
    }
    cnt++;
    life->draw();
  }
}

void Lives::drawNewLife() {}

// Trigger losing a life and erasing a tank sprite
void Lives::loseALife() {
  auto life = tankLives[numLives-1];
  life->erase();
  numLives--;
}

// Trigger gaining a life (happens when all aliens are destroyed); however,
// make sure not to increase lives past the max number of lives.
void Lives::gainALife() {
  if (numLives >= LIVES_MAX) {
    return;
  }

  auto life = tankLives[numLives];
  life->draw();
  numLives++;
}