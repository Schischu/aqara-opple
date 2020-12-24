Overview
========
The DMA memory to memory polling example is a simple demonstration program that uses the SDK software.
It executes one shot polling transfer from source buffer to destination buffer using the SDK DMA drivers.
The purpose of this example is to show how to use the DMA and to provide a simple example for
debugging and further development without DMA interrupt.

Toolchain supported
===================
- IAR embedded Workbench  8.40.2
- MCUXpresso  11.2.0

Hardware requirements
=====================
- Mini USB cable
- OM15076-3 Carrier Board
- JN518x Module plugged on the Carrier Board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a mini USB cable between the PC host and the USB port (J15) on the board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the example runs successfully, the following message is displayed in the terminal:
~~~~~~~~~~~~~~~~~~~~~
DMA memory to memory polling example begin.
Destination Buffer:
0       0       0       0
DMA memory to memory polling example finish.
Destination Buffer:
1       2       3       4
~~~~~~~~~~~~~~~~~~~~~

Customization options
=====================

