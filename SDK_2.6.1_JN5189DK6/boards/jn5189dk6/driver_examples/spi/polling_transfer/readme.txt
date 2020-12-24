Overview
========
The spi_polling example shows how to use spi driver with polling:

In this example, one spi instance as master and another spi instance as slave. Master sends a piece of data to slave,
and check if the data slave received is correct.

Notice: The SPI slave of this example uses interrupt mode, as there is no polling mode for SPI slave.

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
Connect SPI1 pins to SPI0 pins:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Master - SPI1                 CONNECTS TO      Slave - SPI0
Pin Name   Board Location                      Pin Name  Board Location
MISO       J3 pin 25                           MISO      J3 pin 10
MOSI       J3 pin 24                           MOSI      J3 pin 11
SCK        J3 pin 22                           SCK       J3 pin 9
SELN       J3 pin 23                           PCS2      J3 pin 12
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
SPI one board polling example started!

SPI transfer finished!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Customization options
=====================

