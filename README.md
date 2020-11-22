# aqara-opple
aqara-opple


C:\NXP\DK6ProductionFlashProgrammer>DK6Programmer.exe -s COM15 -V2 -m
 COM15: Connected at 115200
 COM15: Detected Unknown JN518x with MAC address 00:00:00:00:00:00:00:00
 COM15: Chip ID: 0x1140C686
 COM15: Bootloader Version: 0x00000000
 COM15: WARNING: Bootloader in device is out of date. See application note JN-AN-1263 or contact support for update information
 COM15: Available memories: FLASH PSECT pFlash Config EFUSE ROM RAM0 RAM1

C:\NXP\DK6ProductionFlashProgrammer>DK6Programmer.exe -s COM15 -V2 -d FLASH=flash.bin
 COM15: Connected at 115200
 COM15: Detected Unknown JN518x with MAC address 00:00:00:00:00:00:00:00
 COM15: Chip ID: 0x1140C686
 COM15: Bootloader Version: 0x00000000
 COM15: WARNING: Bootloader in device is out of date. See application note JN-AN-1263 or contact support for update information
 COM15: Selected memory: FLASH
 COM15: Error: READ access not allowed on FLASH memory
 
 C:\NXP\DK6ProductionFlashProgrammer>DK6Programmer.exe -s COM15 -V2 -d PSECT=psect.bin
 COM15: Connected at 115200
 COM15: Detected Unknown JN518x with MAC address 00:00:00:00:00:00:00:00
 COM15: Chip ID: 0x1140C686
 COM15: Bootloader Version: 0x00000000
 COM15: WARNING: Bootloader in device is out of date. See application note JN-AN-1263 or contact support for update information
 COM15: Selected memory: PSECT
 COM15: Error: READ access not allowed on PSECT memory

C:\NXP\DK6ProductionFlashProgrammer>DK6Programmer.exe -s COM15 -V2 -d pFlash=pflash.bin
 COM15: Connected at 115200
 COM15: Detected Unknown JN518x with MAC address 00:00:00:00:00:00:00:00
 COM15: Chip ID: 0x1140C686
 COM15: Bootloader Version: 0x00000000
 COM15: WARNING: Bootloader in device is out of date. See application note JN-AN-1263 or contact support for update information
 COM15: Selected memory: pFlash
 COM15: Error: READ access not allowed on pFlash memory
 
C:\NXP\DK6ProductionFlashProgrammer>DK6Programmer.exe -s COM15 -V2 -d Config=config.bin
 COM15: Connected at 115200
 COM15: Detected Unknown JN518x with MAC address 00:00:00:00:00:00:00:00
 COM15: Chip ID: 0x1140C686
 COM15: Bootloader Version: 0x00000000
 COM15: WARNING: Bootloader in device is out of date. See application note JN-AN-1263 or contact support for update information
 COM15: Selected memory: Config
 COM15: Error: READ access not allowed on Config memory

C:\NXP\DK6ProductionFlashProgrammer>DK6Programmer.exe -s COM15 -V2 -d EFUSE=efuse.bin
 COM15: Connected at 115200
 COM15: Detected Unknown JN518x with MAC address 00:00:00:00:00:00:00:00
 COM15: Chip ID: 0x1140C686
 COM15: Bootloader Version: 0x00000000
 COM15: WARNING: Bootloader in device is out of date. See application note JN-AN-1263 or contact support for update information
 COM15: Selected memory: EFUSE
 COM15: Dumping EFUSE into file efuse_00-00-00-00-00-00-00-00.bin
 COM15: Dumping EFUSE
 COM15: 0
 COM15: Dumping EFUSE
 COM15: 100
 COM15: Memory dumped successfully
 
 C:\NXP\DK6ProductionFlashProgrammer>DK6Programmer.exe -s COM15 -V2 -d ROM=rom.bin
 COM15: Connected at 115200
 COM15: Detected Unknown JN518x with MAC address 00:00:00:00:00:00:00:00
 COM15: Chip ID: 0x1140C686
 COM15: Bootloader Version: 0x00000000
 COM15: WARNING: Bootloader in device is out of date. See application note JN-AN-1263 or contact support for update information
 COM15: Selected memory: ROM
 COM15: Dumping ROM into file rom_00-00-00-00-00-00-00-00.bin
 COM15: Dumping ROM
 COM15: 0
  COM15: Dumping ROM
 COM15: 100
 COM15: Memory dumped successfully
 
 C:\NXP\DK6ProductionFlashProgrammer>DK6Programmer.exe -s COM15 -V2 -d RAM0=ram0.bin
 COM15: Connected at 115200
 COM15: Detected Unknown JN518x with MAC address 00:00:00:00:00:00:00:00
 COM15: Chip ID: 0x1140C686
 COM15: Bootloader Version: 0x00000000
 COM15: WARNING: Bootloader in device is out of date. See application note JN-AN-1263 or contact support for update information
 COM15: Selected memory: RAM0
 COM15: Error: READ access not allowed on RAM0 memory


C:\NXP\DK6ProductionFlashProgrammer>DK6Programmer.exe -s COM15 -V2 -d RAM1=ram1.bin
 COM15: Connected at 115200
 COM15: Detected Unknown JN518x with MAC address 00:00:00:00:00:00:00:00
 COM15: Chip ID: 0x1140C686
 COM15: Bootloader Version: 0x00000000
 COM15: WARNING: Bootloader in device is out of date. See application note JN-AN-1263 or contact support for update information
 COM15: Selected memory: RAM1
 COM15: Error: READ access not allowed on RAM1 memory


  Alias           Bytes Description

  install_code       48 ZigBee install code (maximum of 36 bytes needed, pad
                        the remaining bytes with 0s)
  zigbee_password    16 ZigBee password
  image_key_valid     4 Image public key status:
                          0: not valid
                          1: valid, unencrypted
                          2: valid, encrypted
                        When reading back, value will be 0 or 2
  image_key_value   256 Image public key, for image authentication level 1 or
                        2 (see auth_level)
  reserved_page0_0   16 Reserved for future use
  reserved_page0_1   16 Reserved for future use
  reserved_page0_2   16 Reserved for future use
  reserved_page0_3   16 Reserved for future use
  reserved_page0_4   16 Reserved for future use
  reserved_page0_5   16 Reserved for future use
  reserved_page0_6   16 Reserved for future use
  reserved_page0_7   16 Reserved for future use
  image_dir_0         8 Image directory (overlaps reserved space)
  image_dir_1         8 Image directory (overlaps reserved space)
  image_dir_2         8 Image directory (overlaps reserved space)
  image_dir_3         8 Image directory (overlaps reserved space)
  image_dir_4         8 Image directory (overlaps reserved space)
  image_dir_5         8 Image directory (overlaps reserved space)
  image_dir_6         8 Image directory (overlaps reserved space)
  image_dir_7         8 Image directory (overlaps reserved space)
  hw_test_mode_dis    4 Hardware test mode: 0 means enabled, any other value
                        means disabled
  isp_level           4 ISP access level:
                          0:           full access, unsecure
                          0x01010101:  full access, secure
                          0x02020202:  write only, unsecure
                          0x03030303:  write only, secure
                          0x04040404:  locked
                          Other value: ISP access disabled
  auth_level          2 Image authentication level, determines checks
                        performed during boot:
                          0: header validity only
                          1: signature of whole image if image has changed
                          2: signature of whole image on every cold start
  unlock_key_valid    2 Unlock key validity:
                          0:   unlock key field is not valid
                          >=1: unlock key field is valid
  app_search_size     4 Application search granularity (increment), in bytes:
                        value of 0 is equated to 4096, other values are used
                        directly
  154_fmac0           8 Factory 802.15.4 MAC address 0
  ble_fmac0           8 Factory BLE MAC address 0
  154_cmac0           8 Custom 802.15.4 MAC address (overrides factory address
                        154_fmac0)
  154_cmac1           8 Custom 802.15.4 MAC address (if second address is
                        required)
  ble_cmac0           8 Custom BLE MAC address (overrides factory address
                        ble_fmac0)
  ble_cmac1           8 Custom BLE MAC address (if second address is required)
  reserved_pFlash_0  16 Reserved for future use
  reserved_pFlash_1  16 Reserved for future use
  reserved_pFlash_2  16 Reserved for future use
  customer_0         16 For customer use
  customer_1         16 For customer use
  customer_2         16 For customer use
  cust_id             8 Customer ID, used for secure handshake
  min_device_id       8 Minimum device ID, used for secure handshake
  device_id           8 Device ID, used for secure handshake
  max_device_id       8 Maximum device ID, used for secure handshake
  unlock_key_value  256 2048-bit public key for secure handshake (equivalent
                        to unlock key)
  swd_jtag_disable    1 Sets the SWD_DIS and JTAG_DIS fields in N-2 page,
                        possible values:
                          0x80: SWD_DIS=1
                          0x70: JTAG_DIS=1
                          0xF0: SWD_DIS=1 and JTAG_DIS=1
                          
                          
C:\NXP\DK6ProductionFlashProgrammer>DK6Programmer.exe -s COM15 -V2 -w isp_level=0x00000000
 COM15: Connected at 115200
 COM15: Detected Unknown JN518x with MAC address 00:00:00:00:00:00:00:00
 COM15: Chip ID: 0x1140C686
 COM15: Bootloader Version: 0x00000000
 COM15: WARNING: Bootloader in device is out of date. See application note JN-AN-1263 or contact support for update information
 COM15: Selected memory: pFlash
 COM15: No READ access needed on pFlash memory

 COM15: Error: WRITE access not allowed on pFlash memory
                          
C:\NXP\DK6ProductionFlashProgrammer>DK6Programmer.exe -s COM15 -V2 -w ble_fmac0=0x0000000000000000
 COM15: Connected at 115200
 COM15: Detected Unknown JN518x with MAC address 00:00:00:00:00:00:00:00
 COM15: Chip ID: 0x1140C686
 COM15: Bootloader Version: 0x00000000
 COM15: WARNING: Bootloader in device is out of date. See application note JN-AN-1263 or contact support for update information
 COM15: Selected memory: Config
 COM15: Error: READ access not allowed on Config memory

 COM15: Error: WRITE access not allowed on Config memory
 
 C:\NXP\DK6ProductionFlashProgrammer>DK6Programmer.exe -s COM15 -V2 -e FLASH
 COM15: Connected at 115200
 COM15: Detected Unknown JN518x with MAC address 00:00:00:00:00:00:00:00
 COM15: Chip ID: 0x1140C686
 COM15: Bootloader Version: 0x00000000
 COM15: WARNING: Bootloader in device is out of date. See application note JN-AN-1263 or contact support for update information
 COM15: Selected memory: FLASH
 COM15: Erasing FLASH
 COM15: Full erase required on memory FLASH, addr=0x00000000, length=646656
 COM15: Erasing FLASH
 COM15: 0
 COM15: Completed
 
 C:\NXP\DK6ProductionFlashProgrammer>DK6Programmer.exe -s COM15 -V2 -p FLASH=DimmerSwitch_OM15082.bin
 COM15: Connected at 115200
 COM15: Detected Unknown JN518x with MAC address 00:00:00:00:00:00:00:00
 COM15: Chip ID: 0x1140C686
 COM15: Bootloader Version: 0x00000000
 COM15: WARNING: Bootloader in device is out of date. See application note JN-AN-1263 or contact support for update information
 COM15: Selected memory: FLASH
 COM15: Programming FLASH at 0x0
 COM15: Full erase required on memory FLASH, addr=0x00000000, length=646656
 COM15: Erasing FLASH
 COM15: 0
 COM15: Completed
 COM15: Programming FLASH
 COM15: 0
  COM15: Programming FLASH
 COM15: 100
 COM15: Memory programmed successfully
