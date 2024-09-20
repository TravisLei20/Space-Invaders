# Space Invaders

## Description
I created a Space Invaders project using a PYNQ board, where I wrote custom drivers in C to connect the switches, buttons, and sounds. As the player progresses, the game speeds up, adding more challenge and excitement. I also implemented a feature to track high scores, giving it a competitive edge and encouraging replayability. This project allowed me to combine hardware and software integration, creating a fun and interactive gaming experience primarily using C on the PYNQ platform.


## Format
 
 * **device_tree:** This folder contains the Linux Device Tree, which enumerates all hardware available to the Linux system.  You won't need to modify this until Lab 6, but some of the information in the [device tree source](device_tree/dts/board_ecen427.dtsi) may be helpful to you for Lab 2 and 5.
  * **hw:** This folder contains the hardware design programmed on the FPGA.  You will be modifying the hardware design in Lab 5, and you won't need to access these files before then. 
  * **kernel:** This folder will contain kernel software, ie. kernel drivers.  You won't need to modify the files here until you create a kernel driver in later labs.
  * **userspace:** This folder contains all software, drivers and applications, that will run in userspace.  You will start working here in Lab 1.
  
  * **format.py**: You can run this program (``./format.py``) to format your software code using clang-format. 
