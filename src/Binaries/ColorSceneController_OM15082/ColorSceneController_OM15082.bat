@echo off
echo ColorSceneController_OM15082.bat
set /p ComPort=Place device into ISP then enter COM port number:
echo Programming ColorSceneController_OM15082.bin...
..\..\..\..\DK6ProductionFlashProgrammer\DK6Programmer -Y -N -V 0 -P 921600 -s COM%ComPort% -e FLASH -p FLASH=ColorSceneController_OM15082.bin 
pause