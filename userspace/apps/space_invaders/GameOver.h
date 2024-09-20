#include <cstdint>
#include <iostream>

#include "Globals.h"
#include "Graphics.h"
#include "HighScores.h"
#include "buttons/buttons.h"
#include "intc/intc.h"
#include "system.h"
#include <iomanip>
#include <sstream>

using namespace std;

const int EXIT_ERROR = -1;
const int DEBOUNCE_WAIT_TIME = 5;
const int FLASH_TIME = 50;
const int NEEDED_CONFIRMS = 3;
// const int ASCII_CONVERSION = 65;
const string ASCII_CONVERSION[] = {"A", "B", "C", "D", "E", "F", "G", "H", "I",
                                   "J", "K", "L", "M", "N", "O", "P", "Q", "R",
                                   "S", "T", "U", "V", "W", "X", "Y", "Z"};
const uint16_t y = 180;
const uint16_t base_x = 274;
const uint16_t buttons_x[] = {base_x, base_x + 33, base_x + 66};

class GameOver {

public:
  enum class State {
    init_st,
    waiting_for_btn_press_st,
    waiting_for_release_st,
    high_score_state
  };

private:
  State currentState;
  long counter;
  long debounce_counter;
  uint8_t button_value;
  int num_of_confirms;
  rgb_t black = {0x00, 0x00, 0x00};
  rgb_t white = {0xFF, 0xFF, 0xFF};
  rgb_t green = {0x00, 0xFF, 0x00};
  int letters[3] = {0, 0, 0};
  int current_letter = 0;
  bool on;
  bool button_handled = false;
  uint32_t score;

public:
  bool finished = false;

  // Constructor
  GameOver(uint32_t score) : currentState(State::init_st) {
    this->score = score;
  }

  void init() {
    counter = 0;
    debounce_counter = 0;
    currentState = State::init_st;
    num_of_confirms = 0;

    int32_t err;

    // Initialize intc
    err = intc_init(SYSTEM_INTC_UIO_FILE);
    if (err) {
      cerr << "intc_init failed" << endl;
      exit(EXIT_ERROR);
    }

    // Initialize buttons
    err = buttons_init(SYSTEM_BUTTONS_UIO_FILE);
    if (err) {
      cerr << "buttons_init failed" << endl;
      exit(EXIT_ERROR);
    }

    // Enable the GPIO interrupt outputs
    buttons_enable_interrupts();

    // Enable the buttons and switches interrupt lines to the interrupt
    // controller
    intc_irq_enable(SYSTEM_INTC_IRQ_BUTTONS_MASK);
    intc_irq_enable(SYSTEM_INTC_IRQ_FIT_MASK);
    intc_enable_uio_interrupts();
  }

  void btn_interrupt() {
    button_value = buttons_read();
    debounce_counter = 0;
    buttons_ack_interrupt();
  }

  void initialize_screen() {
    Globals::getGraphics().fillScreen(black);
    Globals::getGraphics().drawStrCentered("GAME OVER", 50, 10, white);
    Globals::getGraphics().drawStrCentered("ENTER YOUR NAME", 150, 3, white);

    draw_letter(0, white);
    draw_letter(1, white);
    draw_letter(2, white);
    on = true;
  }

  void draw_high_score_screen() {
    printf("Drawing highscore screen\n");

    // erase
    Globals::getGraphics().drawStrCentered("ENTER YOUR NAME", 150, 3, black);
    draw_letter(0, black);
    draw_letter(1, black);
    draw_letter(2, black);

    // draw
    Globals::getGraphics().drawStrCentered("HIGH SCORES", 150, 3, white);

    printf("High scores\n");

    // convert user id into a string
    std::string str;
    str += ASCII_CONVERSION[letters[0]];
    str += ASCII_CONVERSION[letters[1]];
    str += ASCII_CONVERSION[letters[2]];
    str += "\0";
    printf("Score is %d\n", score);

    // Create HighScores object, which stores the player's initials and score
    HighScores high_scores(score, str);

    // Grab the executable filepath
    // Locate/create the text file to store high scores
    // Read in the values from the text file into a vector
    high_scores.init();

    // Add the recent player name and scores into the vector
    // Sort the vector and limit the size
    high_scores.updateHighScores();

    // Repopulate the high scores text file
    high_scores.save();

    // print high scores to screen
    uint16_t y = 180;
    uint16_t x_name = 241;
    uint16_t x_score = x_name + 60;

    // For each entry in the high scores vector,
    for (const auto &entry : high_scores.all_high_scores) {

      // draw name
      Globals::getGraphics().drawStr(entry.first, x_name, y, 3, white);

      // convert score to string and draw
      std::stringstream ss;
      ss << std::setw(5) << std::setfill('0') << entry.second;
      std::string s = ss.str();
      Globals::getGraphics().drawStr(s, x_score, y, 3, green);

      // update y position for next high scores entry
      y += 24;
    }
  }

  // Converts the values 0-25 (which the buttons iterate through upon presses)
  // into letters of alphabet
  void draw_letter(int index, rgb_t color) {
    Globals::getGraphics().drawStr(ASCII_CONVERSION[letters[index]],
                                   buttons_x[index], y, 5, color);
  }

  void state_machine() {
    // Your state machine logic
    switch (currentState) {
    // Initial state
    case State::init_st:
      init();
      currentState = State::waiting_for_btn_press_st;
      break;

    // Wait for button press
    case State::waiting_for_btn_press_st:
      if (counter >= FLASH_TIME) {
        if (on) {
          draw_letter(num_of_confirms, black);
          on = false;
        } else {
          draw_letter(num_of_confirms, white);
          on = true;
        }
        counter = 0;
      }

      if (button_value & BUTTONS_0_MASK || button_value & BUTTONS_1_MASK ||
          button_value & BUTTONS_2_MASK) {
        currentState = State::waiting_for_release_st;
      }
      break;

    // Wait for release after button press
    case State::waiting_for_release_st:
      if (button_handled) {
        if (!button_value) {
          currentState = State::waiting_for_btn_press_st;
          button_handled = false;
        }

      } else if (debounce_counter >= DEBOUNCE_WAIT_TIME) {
        if (button_value & BUTTONS_0_MASK) {
          // draw the confirmed letter
          draw_letter(num_of_confirms, white);
          num_of_confirms++;
        }
        if (button_value & BUTTONS_1_MASK) {
          // down letter logic
          // erase the old letter
          draw_letter(num_of_confirms, black);

          if (letters[num_of_confirms] == 0) {
            letters[num_of_confirms] = 25;
          } else {
            letters[num_of_confirms]--;
          }
          // draw the new letter
          draw_letter(num_of_confirms, white);
        }
        if (button_value & BUTTONS_2_MASK) {
          // up letter logic
          // erase the old letter
          draw_letter(num_of_confirms, black);

          if (letters[num_of_confirms] == 25) {
            letters[num_of_confirms] = 0;
          } else {
            letters[num_of_confirms]++;
          }

          // draw the new letter
          draw_letter(num_of_confirms, white);
        }

        button_handled = true;

        if (num_of_confirms >= NEEDED_CONFIRMS) {
          currentState = State::high_score_state;
          draw_high_score_screen();
        }
        if (!button_value) {
          currentState = State::waiting_for_btn_press_st;
          button_handled = false;
        }
      }
      break;

    // High score state
    case State::high_score_state:
      // Implementation for high_score_state
      break;

    // Handle default case
    default:
      cout << "Default Error Message" << endl;
      break;
    }

    // Perform state action next.
    // Perform the Moore actions based on currentState
    switch (currentState) {
    case State::init_st:
      break;

    case State::waiting_for_btn_press_st:
      counter++;
      break;

    case State::waiting_for_release_st:
      debounce_counter++;
      break;

    case State::high_score_state:
      finished = true;
      break;

    // Handle default case
    default:
      cout << "Default Error Message" << endl;
      break;
    }
  }

  // State machine logic
  void gameoverControl_tick() {

    // Call interrupt controller function to wait for interrupt
    uint32_t interrupts = intc_wait_for_interrupt();

    // Check which interrupt lines are high and call the appropriate ISR
    // functions
    if (interrupts & SYSTEM_INTC_IRQ_FIT_MASK)
      state_machine();
    if (interrupts & SYSTEM_INTC_IRQ_BUTTONS_MASK)
      btn_interrupt();

    // Acknowledge the intc interrupt
    // Re-enable UIO interrupts
    intc_ack_interrupt(interrupts);
    intc_enable_uio_interrupts();
  }
};
