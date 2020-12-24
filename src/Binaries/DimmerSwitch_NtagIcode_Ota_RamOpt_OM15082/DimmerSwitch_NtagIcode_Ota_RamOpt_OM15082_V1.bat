@echo off
echo DimmerSwitch_NtagIcode_Ota_RamOpt_OM15082_V1.bat
set /p ComPort=Place device into ISP then enter COM port number:
echo Programming DimmerSwitch_NtagIcode_Ota_RamOpt_OM15082_V1.bin...
..\..\..\..\DK6ProductionFlashProgrammer\DK6Programmer -Y -N -V 0 -P 921600 -s COM%ComPort% -e FLASH -p FLASH=DimmerSwitch_NtagIcode_Ota_RamOpt_OM15082_V1.bin 
pause