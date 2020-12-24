Overview
========
The spi_interrupt_b2b_master example shows how to use spi functional API to do interrupt transfer as a master:

In this example, the spi instance as master. Master sends a piece of data to slave, receive data from slave,and check
if the data master received is correct. This example needs to work with spi_interrupt_b2b_slave example.

Toolchain supported
===================
- IAR embedded Workbench  8.40.2
- MCUXpresso  11.2.0

Hardware requirements
=====================
- 2 Mini USB cable
- 2 OM15076-3 Carrier Board
- 2 JN518x Module plugged on the Carrier Board
- Personal Computer

Board settings
==============
Connect SPI master on board to SPI slave on other board
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Pin Name   Board Location
MISO       J3 pin 10
MOSI       J3 pin 11
SCK        J3 pin 9
SEL        J3 pin 12

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
4.  Reset the SoC and run the project.

Running the demo
================
The following lines are printed to the serial terminal when the demo program is executed.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SPI board to board interrupt master example started!

SPI transfer finished!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Customization options
=====================

