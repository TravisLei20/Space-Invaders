#include "Aliens.h"
#include "Audio.h"
#include "Bullets.h"
#include "Bunker.h"
#include "Bunkers.h"
#include "GameObject.h"
#include "GameOver.h"
#include "Globals.h"
#include "Graphics.h"
#include "Lives.h"
#include "Score.h"
#include "Sprite.h"
#include "Sprites.h"
#include "Tank.h"
#include "UFO.h"
#include "buttons/buttons.h"
#include "intc/intc.h"
#include "resources/sprites.h"
#include "switches/switches.h"
#include "system.h"
#include <csignal>
#include <cstdlib>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

void initialize_interrupts();
void initialize_audio();
void signalHandler(int signal);

int main() {
  printf("Space Invaders!\n");

  // Set up signal handler for Ctrl+C (SIGINT)
  std::signal(SIGINT, signalHandler);

  // reset screen
  Globals::getGraphics().fillScreen(Globals::getBackgroundColor());
  Globals::getGraphics().drawGround();

  // create game objects
  Tank tank;
  UFO ufo;
  Aliens aliens;
  Bunkers bunkers;
  Globals::getLives().draw();
  Globals::getScore().draw();

  initialize_interrupts();
  initialize_audio();

  // play game
  while ((!Globals::getLives().isGameOver()) && (!aliens.getReachedBunker())) {
    if (aliens.getNumAlive() == 0) {
      Globals::getLives().gainALife();
      aliens.resurrect_aliens();
    }
    uint32_t interrupts = intc_wait_for_interrupt();

    if (interrupts & SYSTEM_INTC_IRQ_PIT_MASK) {
      tank.tick();
      ufo.tick();
      aliens.tick();
      Globals::getBullets().tick();
      bunkers.checkCollisions();
      tank.checkCollisions();
      ufo.checkCollisions();
    }

    if (interrupts & SYSTEM_INTC_IRQ_BUTTONS_MASK) {
      tank.btn_interrupt();
      buttons_ack_interrupt();
    }

    intc_ack_interrupt(interrupts);
    intc_enable_uio_interrupts();
  }

  // clean up
  Globals::getAudio().audio_close();

  // END OF GAME STUFF
  // uint32_t score = 1000;
  GameOver gameOver(Globals::getScore().getScore());
  gameOver.init();
  gameOver.initialize_screen();

  while (!gameOver.finished) {
    gameOver.gameoverControl_tick();
  }

  return 0;
}

void initialize_interrupts() {

  int32_t err;
  err = intc_init(SYSTEM_INTC_UIO_FILE);
  if (err) {
    printf("intc_init failed\n");
    exit(EXIT_ERROR);
  }

  err = buttons_init(SYSTEM_BUTTONS_UIO_FILE);
  if (err) {
    printf("buttons_init failed\n");
    exit(EXIT_ERROR);
  }

  err = switches_init(SYSTEM_SWITCHES_UIO_FILE);
  if (err) {
    printf("switches_init failed\n");
    exit(EXIT_ERROR);
  }

  buttons_enable_interrupts();
  switches_enable_interrupts();

  intc_irq_enable(SYSTEM_INTC_IRQ_BUTTONS_MASK);
  intc_irq_enable(SYSTEM_INTC_IRQ_PIT_MASK);
  intc_irq_enable(SYSTEM_INTC_IRQ_SWITCHES_MASK);

  intc_enable_uio_interrupts();
  printf("Interrupts enabled\n");
}

void initialize_audio() {
  if (!Globals::getAudio().audio_init()) {
    std::cout << "Failed to initialize audio" << std::endl;
    std::exit(EXIT_FAILURE);
  }
}

// Signal handler function
void signalHandler(int signal) {
  std::cout << std::endl << "Ctrl+C received, exiting..." << std::endl;
  Globals::getAudio().audio_close();
  std::exit(EXIT_SUCCESS);
}