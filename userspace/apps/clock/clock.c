#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "./../../drivers/buttons/buttons.h"
#include "./../../drivers/switches/switches.h"
#include "./../../drivers/system.h"
#include "./../../drivers/intc/intc.h"

#define EXIT_ERROR -1
#define DEBOUNCE_WAIT_TIME 3
#define RAPID_INCREMENT_START_TIME 50
#define RAPID_INCREMENT_TIME 10
#define COUNTER_TO_SECOND_TIME 100

volatile uint8_t debounce_count = 0;
volatile uint8_t button_value = 0x0;
volatile uint8_t switches_value = 0x8;

volatile int8_t minute = 0;
volatile int8_t second = 0;
volatile int8_t hour = 0;

volatile int8_t old_hour = 23;
volatile int8_t old_minute = 59;
volatile int8_t old_second = 59;

volatile bool incremented_once = false;

volatile uint8_t counter = 0;
volatile uint8_t rapid_counter = 0;

volatile bool running;
volatile uint8_t increment;


void increment_time_correct() {
  if (second >= 60) {
    second = 0;
    minute++;
  }
  if (minute >= 60) {
    minute = 0;
    hour++;
  }
  if (hour >= 24) {
    hour = 0;
  }
}

void check_time_is_correct() {
  if (second >= 60) {
    second = 0;
  } else if (second < 0) {
    second = 59;
    counter = 0;
  }
  if (minute >= 60) {
    minute = 0;
  } else if (minute < 0) {
    minute = 59;
  }
  if (hour >= 24) {
    hour = 0;
  } else if (hour < 0) {
    hour = 23;
  }
}


// This is invoked in response to a timer interrupt.
// It does 2 things: 1) help debounce buttons, and 2) advances the time.
void isr_fit() {
  debounce_count++;
  counter++;

  // increment second
  if (counter >= COUNTER_TO_SECOND_TIME && running) {
    second++;
    counter = 0;
    increment_time_correct();
  }

  // increment one spot after debounce time
  if (debounce_count >= DEBOUNCE_WAIT_TIME && !incremented_once) {
    if (button_value & BUTTONS_0_MASK) {
      second += increment;
    }
    if (button_value & BUTTONS_1_MASK) {
      minute += increment;
    }
    if (button_value & BUTTONS_2_MASK) {
      hour += increment;
    }
    check_time_is_correct();
    incremented_once = true;
  }

  // rapid increment after btn pressed for .5 sec (500ms/50 iterations)
  if (debounce_count >= RAPID_INCREMENT_START_TIME) {
    rapid_counter++;
    if (rapid_counter >= RAPID_INCREMENT_TIME) {
      if (button_value & BUTTONS_0_MASK) {
        second += increment;
      }
      if (button_value & BUTTONS_1_MASK) {
        minute += increment;
      }
      if (button_value & BUTTONS_2_MASK) {
        hour += increment;
      }
      check_time_is_correct();
      rapid_counter = 0;
    }
  }

  if (hour != old_hour || minute != old_minute || second != old_second) {
    printf("\r%02d:%02d:%02d", hour, minute, second);
    fflush(stdout);
    old_hour = hour;
    old_minute = minute;
    old_second = second;
  }
}

// This is invoked each time there is a change in the button state (result of a
// push or a bounce).
void isr_buttons() {

  button_value = buttons_read();
  incremented_once = false;
  debounce_count = 0;
  buttons_ack_interrupt();

}

void isr_switches() {
  uint8_t switches = switches_read();
  if (switches != switches_value) {
    switches_value = switches;
    if (switches & SWITCHES_0_MASK) {
      increment = 1;
    } else {
      increment = -1;
    }
    if (switches & SWITCHES_1_MASK) {
      running = true;
      counter = 0;
    } else {
      running = false;
      counter = 0;
    }
  }
}


// Run the clock application
int main() {
  // Initialize interrupt controller driver
  // Initialize buttons

  int32_t err;

  // Initialize intc
  err = intc_init(SYSTEM_INTC_UIO_FILE);
  if (err) {
    printf("intc_init failed\n");
    exit(EXIT_ERROR);
  }

  // Initialize buttons
  err = buttons_init(SYSTEM_BUTTONS_UIO_FILE);
  if (err) {
    printf("buttons_init failed\n");
    exit(EXIT_ERROR);
  }

  // Initialize switches
  err = switches_init(SYSTEM_SWITCHES_UIO_FILE);
  if (err) {
    printf("switches_init failed\n");
    exit(EXIT_ERROR);
  }

  // Enable the GPIO interrupt outputs
  buttons_enable_interrupts();
  switches_enable_interrupts();

  // Enable the buttons and switches interrupt lines to the interrupt controller
  intc_irq_enable(SYSTEM_INTC_IRQ_BUTTONS_MASK);
  intc_irq_enable(SYSTEM_INTC_IRQ_SWITCHES_MASK);
  intc_irq_enable(SYSTEM_INTC_IRQ_FIT_MASK);

  isr_switches();

  printf("Clock Starting\n\n");

  while (1) {
    // Call interrupt controller function to wait for interrupt
    uint32_t interrupts = intc_wait_for_interrupt();

    // Check which interrupt lines are high and call the appropriate ISR
    // functions
    if (interrupts & SYSTEM_INTC_IRQ_FIT_MASK)
      isr_fit();
    if (interrupts & SYSTEM_INTC_IRQ_BUTTONS_MASK)
      isr_buttons();
    if (interrupts & SYSTEM_INTC_IRQ_SWITCHES_MASK)
      isr_switches();

    // Acknowledge the intc interrupt
    // Re-enable UIO interrupts
    intc_ack_interrupt(interrupts);
    intc_enable_uio_interrupts();
  }

  intc_exit();
  buttons_exit();
  switches_exit();

  return 0;
}
