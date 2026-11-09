# STM32 Industrial Controls

A bench-scale traffic signal controller for the STM32 NUCLEO-G474RE. The C firmware uses STM32 HAL to drive two groups of red, yellow, and green LEDs.

- Automatic mode sequences green, yellow, and all-red states between the two groups.
- A debounced mode button selects automatic, four-way stop, and two-way stop operation.
- A conflicting-green check latches a fault. Holding the mode button clears the fault and returns the controller to automatic operation.

## Build and run

1. In STM32CubeIDE, choose **File > Import > General > Existing Projects into Workspace** and select this repository. The IDE project is named `G474_Traffic_Controller_Final`.
2. Build the **Debug** configuration for the NUCLEO-G474RE.
3. Connect the bench LEDs and mode button according to the pin definitions in `Core/Inc/main.h`. Use a current-limiting resistor for each LED.
4. Connect the board's ST-LINK USB port and start a debug session to program and run the firmware.

The control logic is in `Core/Src/main.c`. The original CubeMX configuration is `G474_Blink.ioc`.
