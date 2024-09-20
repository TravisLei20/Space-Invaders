#include <stdbool.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>

#include "switches.h"
#include "./../system.h"
#include "./../intc/intc.h"

#define SWITCHES_ERROR 1
#define OPEN_ERROR -1

#define SWCH_MMAP_SIZE 0x1000
#define MMAP_OFFSET 0

#define IP_ISR_OFFSET 0x0120
#define IP_IER_OFFSET 0x0128
#define GIER_OFFSET 0x011C

static int f;     
static char *ptr; 
// bool Enable_GPIO_Interrupt = false;

// Initialize the driver
//  devFilePath: The file path to the uio dev file
//  Return: An error code on error, SWITCHES_SUCCESS otherwise
// This must be called before calling any other switches_* functions
//
//  Tip: This function won't be able to open the UIO device file unless the
//  program is run with ''sudo''.  This is easy to forget, so it is helpful to
//  code an error message into this function that says "Did you forget to
//  sudo?", if it cannot open the UIO file.
int32_t switches_init(const char *devFilePath) {
    // open the device
    f = open(devFilePath, O_RDWR);
    if (f == OPEN_ERROR) {
        printf("buttons init error -- did you forget to sudo?\n");
        return SWITCHES_ERROR;
    }

    // memory map the physical address of the hardware into virtual address space
    ptr = mmap(NULL, SWCH_MMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, f, MMAP_OFFSET);
    if (ptr == MAP_FAILED) {
        return SWITCHES_ERROR;
    }

    return SWITCHES_SUCCESS;
}

// Return the current state of the switches
uint8_t switches_read() {
    return *((volatile uint32_t *)(ptr));
}

// Call this on exit to clean up
void switches_exit() {
    munmap(ptr, SWCH_MMAP_SIZE);
    close(f);
}

// Enable GPIO interrupt output
void switches_enable_interrupts() {
    *((volatile uint32_t *)(ptr + IP_IER_OFFSET)) = 1;
    uint32_t myNumber = 0x80000000;
    *((volatile uint32_t *)(ptr + GIER_OFFSET)) = myNumber;
}

// Disable GPIO interrupt output
void switches_disable_interrupts() {
    *((volatile uint32_t *)(ptr + IP_IER_OFFSET)) = 0;
    uint32_t myNumber = 0x00000000;
    *((volatile uint32_t *)(ptr + GIER_OFFSET)) = myNumber;
}

// Return whether an interrupt is pending
bool switches_interrupt_pending() {
    return *((volatile uint32_t *)(ptr + IP_ISR_OFFSET)) == 1;
}

// Acknowledge a pending interrupt
void switches_ack_interrupt() {
    *((volatile uint32_t *)(ptr + IP_ISR_OFFSET)) = 1;
}
