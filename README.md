# Aqara opple - Alternative Firmware

This page describes how to build and update your Aqara Opple Switch with a full Zigbee 3.0 compatible firmware.

## Description
By default the Aqara Opple Light switch runs a very limited Zigbee 3.0 stack. 
It is not visible in most common Zigbee Bridges, like Philips Hue.
While it can be paired with a Zigbee Network, functionality is very limited.

### Original Button Map
This is the button map of the orignal fw

- Key 0: All lights off
- Key 1: All lights on
- Key 2: All lights decrement brightness
- Key 3: All lights increment brightness
- Key 4: No function
- Key 5: No function

Thankfull Aqara decided to not lock the Aqara Opple NXP JN5189 Zigbee chip, and thereby allows the flashing of an alternative firmware.

### New Button Map
Use this if you have a 2/4 button device, or if you want to use your 6 button device with a non hue bridge

- Key 0: Key On Short/Long Press
- Key 1: Key Off Short/Long Press
- Key 2: Key Dimmer+ Short/Long Press
- Key 3: Key Dimmer+ Short/Long Press
- Key 4: Key 4 Short/Long Press
- Key 5: Key 5 Short/Long Press

### New Button Map - Hue compatible 6 buttons
Use this if you have a 6 button device and want to use it with a hue bridge
Note: Due to the Hue bridge only supporting up to 4 keys, some limitations have to be accepted

- Key 0: Key On Short Press
- Key 1: Key Off Short Press
- Key 2: Key On Long Press
- Key 3: Key Off Long Press
- Key 4: Key Dimmer+ Short/Long Press
- Key 5: Key Dimmer+ Short/Long Press

As the Hue App does not support differenciation of Short/Long Press Key 1 and Key 2 will not have any special functionality.
However by using an App like Hue Dynamics it is possible to give all 6 keys different actions. In total 8 actions can be assigned.

## Software
### Base
The SW is based on the JN-AN-1245 Zigbee 3.0 Controller and Switch example code by NXP

### Status
- 6 Keys
- Powered by Battery, Deep Sleep
- Detected as original Hue Dimmer Switch
- Fully compatible with Hue bridge

### Next steps
- Better Pairing. Automatically go to Steering Mode after Flashing
- Allow Factory Reset. Allows repairing without the need to reflash
- Better handling of Deep Sleep. Currently 10sec after keypress. Best would be periodic wakeup to report battery status.
- Battery Status reporting
- Compare to CoinCell example from SDK
- Provide FW for all variant. 2,4,6 buttons. With and without Hue bridge 4 buttons limitation workaround
- OTA

## Hardware
### Pinout
![Pinout](Aqara_Opple_Pinout.png?raw=true "Pinout")

## Flashing
### JTAG Connector Pinout
![JTAG Pinout](Aqara_Opple_JTAG_Pinout.png?raw=true "JTAG Pinout")

### Option A: Soldering (Recommended if you want to develop)
![JTAG Connection A](Aqara_Opple_JTAG_Connection_PCB_A.png?raw=true "JTAG Connection A")

### Option B: Soldering (Recommended if you just want to falsh)
![JTAG Connection B](Aqara_Opple_JTAG_Connection_PCB_B.png?raw=true "JTAG Connection B")

### UartSBee Connection
![JTAG UartSBee](Aqara_Opple_JTAG_Connection_UartSBee.png?raw=true "JTAG UartSBee")

Programming can be done with for example the UartSBee (https://wiki.seeedstudio.com/UartSBee_v5/), but should work with and FTDI FT232RL based adapter.

```
DK6Programmer.exe -s COM15 -V2 -e FLASH -p FLASH=C:\NXP\JN-AN-1245\Binaries\DimmerSwitch_AQARA_OPPLE\DimmerSwitch_AQARA_OPPLE.bin
```


# OTA
TBD
