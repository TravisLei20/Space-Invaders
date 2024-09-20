#ifndef ALIENS_H
#define ALIENS_H

#include <vector>

#include "Alien.h"
#include "config.h"

// Class that tracks all aliens
class Aliens {
public:
  Aliens();

private:
  // All Alien objects, row by row, starting with top row
  std::vector<std::vector<Alien *>> aliens;

  // Tick counter and max ticks for moving the aliens
  uint32_t moveTickCnt;
  uint32_t moveTickMax;

  // Tick counter and max ticks for firing an alien bullet
  uint32_t fireTickCnt;
  uint32_t fireTickMax;

  // Whether the aliens are moving left or right
  bool movingLeft;

  // The number of aliens left alive
  uint16_t numAliensAlive;

  // The number of times the aliens have been reset, used to increment the speed of the aliens
  uint32_t alienResetCount;

  // Whether the aliens have reached the bunkers, which triggers a game over.
  bool reachedBunker;

  // State for aliens
  typedef enum { MOVING_LEFT, MOVING_RIGHT, MOVING_DOWN } state_t;
  state_t state;

  // Variables to store the farthest outer column cooridinates that have aliens
  // in them.
  uint32_t left_column;
  uint32_t right_column;

  // Helper functions to find the first column on either side of the aliens
  // where there is an alien still alive Sets the outer column variables
  void findLeftColumn();
  void findRightColumn();

  // Returns the bottom most alien that is alive in a column, and nullptr if
  // they are all dead.
  Alien *getBottomAlienInColumn();

  // Vector to store which columns have aliens in them
  std::vector<bool> columnsWithAliens;

  // Sound index for the aliens march sound
  uint8_t aliensMarchSoundIndex;

public:
  // Initialize all aliens.  This can be called again when all aliens are dead
  // to reset the alien group.
  void initialize();

  // Draw all the aliens (probably only need to call this at the beginning)
  // void draw(); // Do we even need this?

  // Tick the alien group, and return whether something moved.
  bool tick();

  // Check for collisions between player bullet and aliens, killing player
  // bullet and alien when they are overlapping.
  bool checkCollisions(Alien *alien);

  // Generate a random number of ticks for firing the next alien bullet and
  // store in fireTickMax
  uint32_t generateRandomFireDelay();

  // Handles the aliens firing bullets.
  // Alien bullets will randomly fire between 100 ms and 1 s (MIN_BULLET_TICKS
  // AND MAX_BULLET_TICKS) Checks to make sure there are less than 4 alien
  // bullets at a time before firing. Used in each state of the state machine
  // assuming the fireTickMax has been reached
  void alienFireBullet();

  // Number of aliens alive
  uint16_t getNumAlive() { return numAliensAlive; }

  // Whether the aliens ahve reached the bottom of the screen (gamve over)
  bool getReachedBunker() { return reachedBunker; }

  void resurrect_aliens();

  void decrementAliensAlive() { numAliensAlive--;}
};
#endif /* ALIENS_H */
