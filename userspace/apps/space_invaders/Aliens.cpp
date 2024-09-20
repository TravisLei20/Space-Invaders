#include "Aliens.h"
#include "Alien.h"
#include "Bullets.h"
#include "GameObject.h"
#include "Globals.h"
#include "config.h"
#include <cstdlib>
#include <iostream>
#include <random>

#define ALIENS_Y_INIT 65 // The top coordinate of where the aliens spawn
#define ALIENS_NUM_MAX (ALIENS_ROWS * ALIENS_COLS) // 55 // 5 rows of 11 aliens
#define ALIENS_MOVE_TICKS 50 // 50 fit timer ticks, or 500 ms
// #define ALIENS_MOVE_TICKS 5 // Test speed
#define MIN_BULLET_TICKS 10  // 10 fit timer ticks, or 100 ms
#define MAX_BULLET_TICKS 100 // 100 fit timer ticks, or 1 second

#define GRAPHICS_WIDTH 640
#define GRAPHICS_HEIGHT 480

void aliensMarchSound(uint8_t index);

// Constructor
Aliens::Aliens() {
  moveTickCnt = 0;
  moveTickMax = ALIENS_MOVE_TICKS;
  fireTickCnt = 0;
  fireTickMax = MAX_BULLET_TICKS;
  numAliensAlive = ALIENS_NUM_MAX;
  alienResetCount = 1;
  reachedBunker = false;
  movingLeft = false;
  state = MOVING_RIGHT;
  left_column = ALIENS_LEFT_RIGHT_MARGIN;
  right_column = ALIENS_LEFT_RIGHT_MARGIN +
                 (ALIENS_COLS * (ALIENS_SPACE_GAP_X + ALIENS_WIDTH));

  for (int i = 0; i < ALIENS_COLS; i++) {
    columnsWithAliens.push_back(true);
  }

  initialize();
}

void Aliens::initialize() {
  aliensMarchSoundIndex = 0;

  // Iterate through the aliens vector and construct an alien object at the
  // right coordinates
  for (int row = 0; row < ALIENS_ROWS; row++) {
    std::vector<Alien *>
        alien_row; // Make a new vector of alien pointers for each row
    for (int col = 0; col < ALIENS_COLS; col++) {
      // Top row, 0
      if (row == 0) {
        alien_row.push_back(
            new Alien(Globals::getSprites().getAlien(SPRITE_ALIEN_TOP_IN),
                      Globals::getSprites().getAlien(SPRITE_ALIEN_TOP_OUT),
                      ALIENS_LEFT_RIGHT_MARGIN +
                          (col * (ALIENS_SPACE_GAP_X + ALIENS_WIDTH)),
                      ALIENS_Y_INIT));
      }
      // Middle rows, 1 and 2
      else if ((row > 0) && (row < 3)) {
        alien_row.push_back(new Alien(
            Globals::getSprites().getAlien(SPRITE_ALIEN_MID_IN),
            Globals::getSprites().getAlien(SPRITE_ALIEN_MID_OUT),
            ALIENS_LEFT_RIGHT_MARGIN +
                (col * (ALIENS_SPACE_GAP_X + ALIENS_WIDTH)),
            ALIENS_Y_INIT + (row * (ALIENS_SPACE_GAP_Y + ALIENS_HEIGHT))));
      }
      // Bottom rows, 3 and 4
      else {
        alien_row.push_back(new Alien(
            Globals::getSprites().getAlien(SPRITE_ALIEN_BOT_IN),
            Globals::getSprites().getAlien(SPRITE_ALIEN_BOT_OUT),
            ALIENS_LEFT_RIGHT_MARGIN +
                (col * (ALIENS_SPACE_GAP_X + ALIENS_WIDTH)),
            ALIENS_Y_INIT + (row * (ALIENS_SPACE_GAP_Y + ALIENS_HEIGHT))));
      }
    }
    aliens.push_back(alien_row);
  }

  // Set each of these values to true, indicating that all columns have alive
  // aliens in them
  for (int i = 0; i < ALIENS_COLS; i++) {
    columnsWithAliens[i] = true;
  }
}

// Returns the bottom most alien that is alive in a column, and nullptr if
// they are all dead.
Alien *Aliens::getBottomAlienInColumn() {
  std::vector<uint8_t> columnsRemaining;

  // Check which columns still have aliens in them
  for (int i = 0; i < ALIENS_COLS; i++) {
    if (columnsWithAliens[i]) {
      columnsRemaining.push_back(i);
    }
  }

  // Randomly select which column to shoot from from the ones that still have
  // aliens
  uint8_t randomColumnIndex = rand() % columnsRemaining.size();
  uint8_t randomColumn = columnsRemaining[randomColumnIndex];

  // Find the bottom-most alive alien in the random column
  size_t bottomRow = aliens.size(); // Initialize to the last row
  for (int i = aliens.size() - 1; i >= 0; --i) {
    if (aliens[i][randomColumn]->isAlive()) {
      bottomRow = i;
      break;
    }
  }

  return aliens[bottomRow][randomColumn];
}

// Generate a random number of ticks for firing the next alien bullet and store
// in fireTickMax
uint32_t Aliens::generateRandomFireDelay() {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_int_distribution<uint32_t> dist(MIN_BULLET_TICKS,
                                               MAX_BULLET_TICKS);
  return dist(gen);
}

// Handles the aliens firing bullets.
// Alien bullets will randomly fire between 100 ms and 1 s (MIN_BULLET_TICKS AND
// MAX_BULLET_TICKS) Checks to make sure there are less than 4 alien bullets at
// a time before firing. Used in each state of the state machine assuming the
// fireTickMax has been reached
void Aliens::alienFireBullet() {

  // If the alien bullet array is not already full at 4 bullets, fire a bullet
  // from a random alien column
  if (!Globals::getBullets().enemyBulletsMaxed()) {

    // Create a new alien bullet object
    Globals::getBullets().newEnemyBullet(getBottomAlienInColumn());
  }

  // Reset the fireTickMax and fireTickCnt
  fireTickMax = generateRandomFireDelay();
  fireTickCnt = 0;
}

// Helper functions to find the first column on either side of the aliens where
// there is an alien still alive Sets the outer column variables
void Aliens::findLeftColumn() {
  for (size_t j = 0; j < aliens[0].size(); j++) {
    bool found = false;
    for (size_t i = 0; i < aliens.size(); i++) {
      if (aliens[i][j]->isAlive()) {
        found = true;
        left_column = aliens[i][j]->getX();
        break;
      }
    }
    if (found) {
      break;
    }
  }
}
void Aliens::findRightColumn() {
  for (size_t j = aliens[0].size() - 1; j >= 0; j--) {
    bool found = false;
    for (size_t i = 0; i < aliens.size(); i++) {
      if (aliens[i][j]->isAlive()) {
        found = true;
        right_column = aliens[i][j]->getX();
        break;
      }
    }
    if (found) {
      break;
    }
  }
}

bool Aliens::tick() {

  for (int row = 0; row < ALIENS_ROWS; row++) {
    for (int col = 0; col < ALIENS_COLS; col++) {
      if (checkCollisions(aliens[row][col])) {
        if (row == 4) {
          Globals::getScore().hitBotAlien();
        } else if (row == 0) {
          Globals::getScore().hitTopAlien();
        } else {
          Globals::getScore().hitMidAlien();
        }
      }
      // Tick the individual alien object which handles exploding if necessary
      aliens[row][col]->tick();
    }
  }

  // update columnsWithAliens
  for (int col = 0; col < ALIENS_COLS; col++) {
    for (int row = 0; row < ALIENS_ROWS; row++) {
      if (aliens[row][col]->isAlive()) {
        break;
      }
      if (row == (ALIENS_ROWS - 1)) {
        columnsWithAliens[col] = false;
      }
    }
  }

  // Aliens State Machine, Mealy
  switch (state) {
  case MOVING_LEFT:

    if (left_column <= ALIENS_LEFT_RIGHT_MARGIN) {
      findLeftColumn();
      if (left_column <= ALIENS_LEFT_RIGHT_MARGIN) {
        state = MOVING_DOWN;
      }
    }

    break;

  case MOVING_RIGHT:

    if (right_column >= (GRAPHICS_WIDTH - 2 * ALIENS_LEFT_RIGHT_MARGIN)) {
      findRightColumn();
      if (right_column >= (GRAPHICS_WIDTH - 2 * ALIENS_LEFT_RIGHT_MARGIN)) {
        state = MOVING_DOWN;
      }
    }

    break;

  case MOVING_DOWN:

    break;
  }

  // Aliens State Machine, Moore
  switch (state) {

  case MOVING_LEFT:
    moveTickCnt++;
    fireTickCnt++;

    // Moving after tick count reached
    if (moveTickCnt >= moveTickMax) {
      aliensMarchSound(aliensMarchSoundIndex);
      aliensMarchSoundIndex = (aliensMarchSoundIndex + 1) % 4;
      for (int row = 0; row < ALIENS_ROWS; row++) {
        for (int col = 0; col < ALIENS_COLS; col++) {
          // checkCollisions(aliens[row][col]);
          aliens[row][col]->moveLeft();
          moveTickCnt = 0;
        }
      }
      left_column -= ALIENS_MOVE_X_DISTANCE;
      right_column -= ALIENS_MOVE_X_DISTANCE;
    }

    // Firing after tick count reached
    if (fireTickCnt >= fireTickMax) {
      alienFireBullet();
    }

    break;

  case MOVING_RIGHT:
    moveTickCnt++;
    fireTickCnt++;

    // Moving after tick count reached
    if (moveTickCnt >= moveTickMax) {
      aliensMarchSound(aliensMarchSoundIndex);
      aliensMarchSoundIndex = (aliensMarchSoundIndex + 1) % 4;
      for (int row = 0; row < ALIENS_ROWS; row++) {
        for (int col = 0; col < ALIENS_COLS; col++) {
          // checkCollisions(aliens[row][col]);
          aliens[row][col]->moveRight();
          moveTickCnt = 0;
        }
      }
      left_column += ALIENS_MOVE_X_DISTANCE;
      right_column += ALIENS_MOVE_X_DISTANCE;
    }

    // Firing after tick count reached
    if (fireTickCnt >= fireTickMax) {
      alienFireBullet();
    }

    break;

  case MOVING_DOWN:
    moveTickCnt++;
    fireTickCnt++;

    // Moving after tick count reached
    if (moveTickCnt >= moveTickMax) {
      aliensMarchSound(aliensMarchSoundIndex);
      aliensMarchSoundIndex = (aliensMarchSoundIndex + 1) % 4;
      for (int row = 0; row < ALIENS_ROWS; row++) {
        for (int col = 0; col < ALIENS_COLS; col++) {
          aliens[row][col]->moveDown();
          moveTickCnt = 0;
        }
      }

      if (movingLeft) {
        movingLeft = false;
        state = MOVING_RIGHT;
      } else {
        movingLeft = true;
        state = MOVING_LEFT;
      }
    }
    // Firing after tick count reached
    if (fireTickCnt >= fireTickMax) {
      alienFireBullet();
    }

    break;
  }

  return true; // Lol wut
}

// Check for collisions between player bullet and aliens, killing player bullet
// and alien when they are overlapping.
bool Aliens::checkCollisions(Alien *alien) {
  Bullet *playerBullet = Globals::getBullets().getPlayerBullet();
  if (alien->isOverlapping(playerBullet)) {
    // Erase and remove the player bullet
    playerBullet->kill();
    Globals::getBullets().kill(playerBullet);
    // Erase the alien that was hit and set its alive flag to false
    alien->explode();

    numAliensAlive--;
    return true;
  }

  // Check to see if an alien has reached the bunker area, which sets the flag
  // to go to Game Over in main
  if (alien->getY() >= (BUNKER_Y - ALIENS_HEIGHT)) {
    reachedBunker = true;
  }

  return false;
}

void Aliens::resurrect_aliens() {
  // printf("Resurrecting aliens\n");
  for (int row = 0; row < ALIENS_ROWS; row++) {
    for (int col = 0; col < ALIENS_COLS; col++) {
      Alien *alien = aliens[row][col];
      alien->kill();
      alien->reset();
    }
  }
  for (int row = 0; row < ALIENS_ROWS; row++) {
    for (int col = 0; col < ALIENS_COLS; col++) {
      if (row == 0) {
        Alien *alien = aliens[row][col];
        alien->setX(ALIENS_LEFT_RIGHT_MARGIN +
                    (col * (ALIENS_SPACE_GAP_X + ALIENS_WIDTH)));
        alien->setY(ALIENS_Y_INIT);
        alien->resurrect(alien->getStartingSprite());
      }
      // Middle rows, 1 and 2
      else if ((row > 0) && (row < 3)) {
        Alien *alien = aliens[row][col];
        alien->setX(ALIENS_LEFT_RIGHT_MARGIN +
                    (col * (ALIENS_SPACE_GAP_X + ALIENS_WIDTH)));
        alien->setY(ALIENS_Y_INIT +
                    (row * (ALIENS_SPACE_GAP_Y + ALIENS_HEIGHT)));
        alien->resurrect(alien->getStartingSprite());
      }
      // Bottom rows, 3 and 4
      else {
        Alien *alien = aliens[row][col];
        alien->setX(ALIENS_LEFT_RIGHT_MARGIN +
                    (col * (ALIENS_SPACE_GAP_X + ALIENS_WIDTH)));
        alien->setY(ALIENS_Y_INIT +
                    (row * (ALIENS_SPACE_GAP_Y + ALIENS_HEIGHT)));
        alien->resurrect(alien->getStartingSprite());
      }
    }
  }
  // Reset the number of alive aliens
  numAliensAlive = ALIENS_NUM_MAX;

  // Increase the speed of the aliens
  alienResetCount++;
  if ((alienResetCount * 5) < ALIENS_MOVE_TICKS) {
    moveTickMax = ALIENS_MOVE_TICKS - (5 * alienResetCount);
  }

  // Reset the left-most and right-most column index
  left_column = ALIENS_LEFT_RIGHT_MARGIN;
  right_column = ALIENS_LEFT_RIGHT_MARGIN +
                 (ALIENS_COLS * (ALIENS_SPACE_GAP_X + ALIENS_WIDTH));

  // Make sure this stupid vector is set to all true
  for (int i = 0; i < ALIENS_COLS; i++) {
    columnsWithAliens[i] = true;
  }
}

void aliensMarchSound(uint8_t index) {
  Audio::AudioFile sound;
  switch (index) {
  case 0:
    sound = Globals::getAudio().WALK1;
    break;
  case 1:
    sound = Globals::getAudio().WALK2;
    break;
  case 2:
    sound = Globals::getAudio().WALK3;
    break;
  case 3:
    sound = Globals::getAudio().WALK4;
    break;
  default:
    sound = Globals::getAudio().WALK1;
    break;
  }
  Globals::getAudio().play_audio(sound, false);
}