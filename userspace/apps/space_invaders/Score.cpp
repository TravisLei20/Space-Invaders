#include "Score.h"
#include "Globals.h"
#include "config.h"
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <stdint.h>
#include <string>

#define SCORE_TEXT_WIDTH SPRITES_5X5_COLS
#define SCORE_TEXT_NUM_CHARS 5
#define SCORE_TEXT_SPACE_USED                                                  \
  SCORE_TEXT_NUM_CHARS *SCORE_TEXT_WIDTH *SCORE_SIZE + SCORE_TEXT_NUM_CHARS * 3

std::string Score::padScore(uint32_t score) {
  std::stringstream ss;
  ss << std::setw(SCORE_DIGITS) << std::setfill('0') << score;
  std::string s = ss.str();
  return s;
}

Score::Score() {
  score = 0;
  Globals::getGraphics().drawStr("SCORE", SCORE_X_MARGIN, SCORE_Y_MARGIN,
                                 SCORE_SIZE, {0xFF, 0xFF, 0xFF});
  draw();
}

// Draw the score at game start
void Score::draw() {
  Globals::getGraphics().drawStr("SCORE", SCORE_X_MARGIN, SCORE_Y_MARGIN,
                                 SCORE_SIZE, {0xFF, 0xFF, 0xFF});
  Globals::getGraphics().drawStr(
      Score::padScore(score), SCORE_X_MARGIN + SCORE_TEXT_SPACE_USED,
      SCORE_Y_MARGIN, SCORE_SIZE, {0x00, 0xFF, 0x00});
}

// erase old score in preparation for new score to be drawn
void Score::erase() {
  Globals::getGraphics().drawStr(
      Score::padScore(score), SCORE_X_MARGIN + SCORE_TEXT_SPACE_USED,
      SCORE_Y_MARGIN, SCORE_SIZE, {0x00, 0x00, 0x00});
}
