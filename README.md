# Aqara opple - Alternative Firmware

This page describes how to build and update your Aqara Opple Switch with a full Zigbee 3.0 compatible firmware.

## Description
By default the Aqara Opple Light switch runs a very limited Zigbee 3.0 stack. 
It is not visible in most common Zigbee Bridges, like Philips Hue.
While it can be paired with a Zigbee Network, functionality is very limited.

Per default only following functionality exsists:
- Key 0: All lights off
- Key 1: All lights on
- Key 2: All lights decrement brightness
- Key 3: All lights increment brightness
- Key 4: No function
- Key 5: No function

Thankfull Aqara decided to not lock the Aqara Opple NXP JN5189 Zigbee chip, and thereby allows the flashing of an alternative firmware.

## Hardware
### Pinout
![Pinout](Aqara_Opple_Pinout.png?raw=true "Pinout")

## Software
### Base
The SW is based on the JN-AN-1245 Zigbee 3.0 Controller and Switch example code by NXP

### Mapping
Due to the Hue bridge only supporting up to 4 keys, some limitations have ot be accepted
- Key 0: Key On Short Press
- Key 1: Key Off Short Press
- Key 2: Key On Long Press
- Key 3: Key Off Long Press
- Key 4: Key Dimmer+ Short/Long Press
- Key 5: Key Dimmer+ Short/Long Press

As the Hue App does not support differenciation of Short/Long Press Key 1 and Key 2 will not have any special functionality.
However by using an App like Hue Dynamics it is possible to give all 6 keys different actions. In total 8 actions can be assigned.

## Flashing
### JTAG Connector Pinout
![JTAG Pinout](Aqara_Opple_JTAG_Pinout.png?raw=true "JTAG Pinout")

Programming can be done with for example the UartSBee (https://wiki.seeedstudio.com/UartSBee_v5/), but should work with and FTDI FT232RL based adapter.

```
DK6Programmer.exe -s COM15 -V2 -e FLASH -p FLASH=C:\NXP\JN-AN-1245\Binaries\DimmerSwitch_AQARA_OPPLE\DimmerSwitch_AQARA_OPPLE.bin
```


# OTA
TBD
