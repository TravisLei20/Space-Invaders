#ifndef SCORE_H
#define SCORE_H

#include <cstdlib>
#include <stdint.h>
#include <string>

#include "config.h"

// Kepp track of and draw the game score
class Score {
public:
  Score();

private:
  // The game score
  uint32_t score;

public:
  // Pad a score integer with 0s
  static std::string padScore(uint32_t score);

  uint32_t getScore() { return score; }

  // Draw the score at game start
  void draw();

  // erase old score in preparation for new score to be drawn
  void erase();

  // Call the functions when aliens are hit.
  void hitTopAlien() {
    erase();
    score += SCORE_TOP_ALIEN;
    draw();
  }
  void hitMidAlien() {
    erase();
    score += SCORE_MID_ALIEN;
    draw();
  }
  void hitBotAlien() {
    erase();
    score += SCORE_BOT_ALIEN;
    draw();
  }
  void hitSpaceship() {
    erase();
    score += (rand() % (SCORE_SPACESHIP_MAX - SCORE_SPACESHIP_MIN)) +
             SCORE_SPACESHIP_MIN;
    draw();
  }
};

#endif /* SCORE_H */
