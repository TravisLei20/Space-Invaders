#include <stdbool.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>

#include "intc.h"

#define INTC_ERROR 1
#define OPEN_ERROR -1

#define INTC_MMAP_SIZE 0x1000
#define MMAP_OFFSET 0

#define IAR_OFFSET 0x0C
#define SIE_OFFSET 0x10
#define IPR_OFFSET 0x04
#define CIE_OFFSET 0x14
#define MER_OFFSET 0x1C

#define ISR_OFFSET 0x00

static int f;     
static char *ptr; 

// Initializes the driver (opens UIO file and calls mmap)
// devDevice: The file path to the uio dev file
// Returns: A negative error code on error, INTC_SUCCESS otherwise
// This must be called before calling any other intc_* functions
//
//  Tip: This function won't be able to open the UIO device file unless the
//  program is run with ''sudo''.  This is easy to forget, so it is helpful to
//  code an error message into this function that says "Did you forget to
//  sudo?", if it cannot open the UIO file.
int32_t intc_init(const char devDevice[]) {
    // open the device
    f = open(devDevice, O_RDWR);
    if (f == OPEN_ERROR) {
        printf("intc init error -- did you forget to sudo?\n");
        return INTC_ERROR;
    }

    // memory map the physical address of the hardware into virtual address space
    ptr = mmap(NULL, INTC_MMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, f, MMAP_OFFSET);
    if (ptr == MAP_FAILED) {
        return INTC_ERROR;
    }

    volatile uint32_t *iar = (volatile uint32_t *)(ptr + MER_OFFSET);
    *iar = 3;
    intc_enable_uio_interrupts();

    return INTC_SUCCESS;
}

// Called to exit the driver (unmap and close UIO file)
void intc_exit() {
    munmap(ptr, INTC_MMAP_SIZE);
    close(f);
}

// This function will block until an interrupt occurrs
// Returns: Bitmask of activated interrupts
uint32_t intc_wait_for_interrupt() {
    uint32_t buffer;  // Temporary buffer to store the read value

    // Use a non-volatile pointer for the read function
    read(f, &buffer, sizeof(uint32_t));

    // Return the value read from the interrupt pending register
    return *(volatile uint32_t *)(ptr + IPR_OFFSET);
}

// Acknowledge interrupt(s) in the interrupt controller
// irq_mask: Bitmask of interrupt lines to acknowledge.
void intc_ack_interrupt(uint32_t irq_mask) {
    volatile uint32_t *iar = (volatile uint32_t *)(ptr + IAR_OFFSET);
    // Acknowledge the interrupts by writing 1 to the corresponding bits in the IAR
    *iar = irq_mask;
}

// // Instruct the UIO to enable interrupts for this device in Linux
// // (see the UIO documentation for how to do this)
void intc_enable_uio_interrupts() {
    uint32_t enable_value = 1;
    write(f, &enable_value, sizeof(uint32_t));
}

// Enable interrupt line(s)
// irq_mask: Bitmask of lines to enable
// This function only enables interrupt lines, ie, a 0 bit in irq_mask
//	will not disable the interrupt line
void intc_irq_enable(uint32_t irq_mask) {
    volatile uint32_t *sie = (volatile uint32_t *)(ptr + SIE_OFFSET);
    // Set interrupt enables by writing 1 to the corresponding bits in SIE
    *sie = irq_mask;
}

// Same as intc_irq_enable, except this disables interrupt lines
void intc_irq_disable(uint32_t irq_mask) {
    volatile uint32_t *sie = (volatile uint32_t *)(ptr + CIE_OFFSET);
    // Set interrupt disable by writing 0 to the corresponding bits in SIE
    *sie = irq_mask;
}
