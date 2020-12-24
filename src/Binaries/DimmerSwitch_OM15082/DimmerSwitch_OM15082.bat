@echo off
echo DimmerSwitch_OM15082.bat
set /p ComPort=Place device into ISP then enter COM port number:
echo Programming DimmerSwitch_OM15082.bin...
..\..\..\..\DK6ProductionFlashProgrammer\DK6Programmer -Y -N -V 0 -P 921600 -s COM%ComPort% -e FLASH -p FLASH=DimmerSwitch_OM15082.bin 
pause