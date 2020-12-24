###############################################################################
#
# MODULE:   Makefile
#
# DESCRIPTION: Green Power, Energy Harvesting, Switch
# 
############################################################################
# This software is owned by NXP B.V. and/or its supplier and is protected
# under applicable copyright laws. All rights are reserved. We grant You,
# and any third parties, a license to use this software solely and
# exclusively on NXP products [NXP Microcontrollers such as JN5168, JN5169, 
# JN5179, JN5189].
# You, and any third parties must reproduce the copyright and warranty notice
# and any other legend of ownership on each copy or partial copy of the
# software.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# Copyright NXP B.V. 2012-2019. All rights reserved
#
#############################################################################
#
# Application target name

TARGET = GpEhSwitch
CFLAGS += -DPDM$(PDM_BUILD_TYPE)
DR ?= OM15082
ZBPRO_DEVICE_TYPE ?= ZED
###############################################################################
# User definable make parameters that may be overwritten from the command line
# Default target device is the JN5180

# Set specific chip	(choose one)
JENNIC_CHIP           ?= K32W061
#JENNIC_CHIP           ?= K32W041
#JENNIC_CHIP           ?= JN5189
#JENNIC_CHIP           ?= JN5188

# Legacy chip family (always JN518x)
JENNIC_CHIP_FAMILY     ?= JN518x

# K32W0xx SDK settings
ifeq ($(findstring K32W0,$(JENNIC_CHIP)),K32W0)
# Chip/Device
SDK_DEVICE_FAMILY      ?= K32W061
# Board
SDK_BOARD              ?= k32w061dk6
# SDK folder
SDK_FOLDER             ?= K32W061DK6
endif

# JN518x SDK settings
ifeq ($(findstring JN518,$(JENNIC_CHIP)),JN518)
# Chip/Device
SDK_DEVICE_FAMILY      ?= JN5189
# Board
SDK_BOARD              ?= jn5189dk6
# SDK folder
SDK_FOLDER             ?= JN5189DK6
endif

ENDIAN                 ?= LITTLE_ENDIAN
DISABLE_LTO             = 1
DEBUG                  ?= NONE
HEAP_SIZE              ?= 0x2F4

###############################################################################
# Default DK4 development kit target hardware
#JENNIC_PCB ?= DEVKIT4

CFLAGS += -D$(DR)
CFLAGS += -D$(TARGET)

CFLAGS  += -DBUTTON_MAP_$(DR)
BIN_SUFFIX=_$(DR)
SWITCH_TYPE_$(DR)?= 1

# 32k clock configuration (1 = FRO, 0 = XTAL)
CLK_32K = 1
ifeq ($(CLK_32K), 1)
CFLAGS += -DgClkUseFro32K=1
CFLAGS += -DgPWR_UseAlgoTimeBaseDriftCompensate=1
# 32k FRO automatic calibration (0 = disable, 1 = enable)
CFLAGS += -DgClkRecalFro32K=0
else
CFLAGS += -DgClkUseFro32K=0
endif

###############################################################################
# Framework options
FRAMEWORK_SWITCH = 1

###############################################################################
# Debug options define DEBUG either HW or SW
#DEBUG ?=SW
#DEBUG ?=HW
#
# Define which UART to use for HW debug
DEBUG_PORT ?= UART0

###############################################################################
# Define TRACE to use with DBG module
TRACE ?=1
ifeq ($(TRACE), 1)	
CFLAGS += -DDBG_ENABLE
CFLAGS += -DDBG_DEVICE_SWITCH
CFLAGS  += -DPRINTF_ADVANCED_ENABLE
endif

###############################################################################
# Path definitions
# Use if application directory contains single target

# Toolchain paths
TOOLCHAIN_DIR_PATH  = ../../../../../MCUXpressoIDE_11.2.0_4120/ide/tools
TOOLCHAIN_PATH      = $(TOOLCHAIN_DIR_PATH)/bin

# SDK paths
SDK2_BASE_DIR       =  ../../../../../SDKPackages/$(SDK_FOLDER)

# Application paths
APP_BASE            =  ../../..
APP_BLD_DIR         = $(APP_BASE)/$(TARGET)/Build/jn518x_mcux
APP_SRC_DIR         = $(APP_BASE)/$(TARGET)/Source
APP_COMMON_SRC_DIR  = $(APP_BASE)/Common/Source
BOARD_DIR           = $(APP_BASE)/Common/Source/board
AWK_SRC             = $(APP_BASE)/Common/Awk

# Tool paths
AWK_EXE             = $(APP_BASE)/../../gawk-3.1.6-1-bin/bin/gawk.exe

###############################################################################
# Application Source files

# Note: Path to source file is found using vpath below, so only .c filename is required
APPSRC  += app_main.c
APPSRC  += EH_IEEE_802154_Switch.c
APPSRC  += EH_IEEE_Features.c
APPSRC  += EH_IEEE_Commands.c
APPSRC  += app_nvm.c
APPSRC  += EH_Button.c
APPSRC  += fsl_debug_console.c
APPSRC  += fsl_gpio.c
APPSRC  += fsl_gint.c
APPSRC  += hardware_init.c
APPSRC  += pin_mux.c
APPSRC  += port_JN518x.c
APPSRC += board_utility.c

###############################################################################
# Standard Application header search paths

INCFLAGS += -I$(APP_SRC_DIR)
INCFLAGS += -I$(BOARD_DIR)
INCFLAGS += -I$(APP_COMMON_SRC_DIR)

CFLAGS   += -DTAG_BUILD_RANDOM

###############################################################################
# Application libraries
# Specify additional Component libraries

###############################################################################
# Construct strings for output file names

TARGET_FULL=$(TARGET)$(BIN_SUFFIX)

$(info Creating folders...)
APP_OUT_DIR = $(APP_BASE)/Binaries/$(TARGET_FULL)
APP_OBJ_DIR = $(APP_BASE)/Binaries/$(TARGET_FULL)/obj
$(info $(shell mkdir -p $(APP_OUT_DIR)))
$(info $(shell mkdir -p $(APP_OBJ_DIR)))
$(info APP_OUT_DIR = $(APP_OUT_DIR))
$(info APP_OBJ_DIR = $(APP_OBJ_DIR))

###############################################################################

# You should not need to edit below this line

###############################################################################
###############################################################################
# Configure for the selected chip or chip family
ifeq ($(FRAMEWORK_SWITCH), 1)
include $(SDK2_BASE_DIR)/middleware/wireless/zigbee/BuildConfig/ZBPro/Build/config_ZBPro.mk
else
include $(SDK2_BASE_DIR)/middleware/wireless/zigbee3.0/BuildConfig/ZBPro/Build/config_ZBPro.mk
endif

###############################################################################
TEMP = $(APPSRC:.c=.o)
APPOBJS_TMP = $(TEMP:.S=.o)
APPOBJS := $(addprefix $(APP_OBJ_DIR)/,$(APPOBJS_TMP))

###############################################################################
# Application dynamic dependencies

APPDEPS = $(APPOBJS:.o=.d)
###############################################################################
# Linker

# Add application libraries before chip specific libraries to linker so
# symbols are resolved correctly (i.e. ordering is significant for GCC)
LDLIBS := $(APPLIBS) $(LDLIBS)

IMAGE_TOOL = $(SDK2_BASE_DIR)/tools/imagetool/dk6_image_tool.py
IMAGE_TOOL_CONFIGURATION = -s 294912

###############################################################################
# Dependency rules

.PHONY: all clean

# Path to directories containing application source 
vpath % $(APP_SRC_DIR):$(APP_COMMON_SRC_DIR):$(BOARD_DIR):$(ZIGBEE_BASE_SRC):$(BOARD_DIR)

all: $(APP_OUT_DIR)/$(TARGET_FULL).axf

-include $(APPDEPS)
$(APP_OBJ_DIR)/%.d:
	rm -f $*.o

$(APP_OBJ_DIR)/app_main.o: app_main.c 
	$(info Compiling $< ...)
	$(CC) -c -o $@ $(CFLAGS) $(INCFLAGS) $< -MD -MF $(APP_OBJ_DIR)/$*.d -MP
	@echo "Source Files" > $(APP_OUT_DIR)/$(TARGET_FULL).log
	@echo $@ $< >> $(APP_OUT_DIR)/$(TARGET_FULL).log
	@echo
	
$(APP_OBJ_DIR)/%.o: %.c 
	$(info Compiling $< ...)
	$(CC) -c -o $(subst Source,Build,$@) $(CFLAGS) $(INCFLAGS) $< -MD -MF $(APP_OBJ_DIR)/$*.d -MP
	@echo $@ $< >> $(APP_OUT_DIR)/$(TARGET_FULL).log
	@echo

$(APP_OBJ_DIR)/%.o: %.S
	$(info Assembling $< ...)
	$(CC) -c -o $(subst Source,Build,$@) $(CFLAGS) $(INCFLAGS) $< -MD -MF $(APP_OBJ_DIR)/$*.d -MP
	@echo $@ $< >> $(APP_OUT_DIR)/$(TARGET_FULL).log
	@echo

$(APP_OUT_DIR)/$(TARGET_FULL).axf: $(APPOBJS) $(addsuffix.a,$(addprefix $(COMPONENTS_BASE_DIR)/Library/lib,$(APPLDLIBS))) 
	$(info Linking $@ ...)
	$(TOOLCHAIN_PATH)/$(CC)  -Wl,--gc-sections $(LDFLAGS) -T "AppBuild16kEndDevice_$(JENNIC_CHIP).ld" -o $@ -Wl,--start-group $(APPOBJS) $(addprefix -l,$(LDLIBS)) -Wl,--end-group -Wl,-Map,$(APP_OUT_DIR)/$(TARGET_FULL).map 
	$(info Generating binary ...)
	$(TOOLCHAIN_PATH)/$(SIZE) $@  
	date +%c >> $(APP_BASE)/Doc/size.txt
	$(TOOLCHAIN_PATH)/$(SIZE) $@ >> $(APP_BASE)/Doc/size.txt
	-$(AWK_EXE) -f $(AWK_SRC)/JN51xx_MAP.awk $(APP_OUT_DIR)/$(TARGET_FULL).log $(APP_OUT_DIR)/$(TARGET_FULL).map > $(APP_OBJ_DIR)/temp.txt
	$(IMAGE_TOOL) $(IMAGE_TOOL_CONFIGURATION) $(APP_OUT_DIR)/$(TARGET_FULL).axf 
	rm -f $(APP_BLD_DIR)/data.bin
	$(TOOLCHAIN_PATH)/$(OBJCOPY) -v -O binary $(APP_OUT_DIR)/$(TARGET_FULL).axf $(APP_OUT_DIR)/$(TARGET_FULL).bin
	$(TOOLCHAIN_PATH)/$(OBJDUMP) -d $(APP_OUT_DIR)/$(TARGET_FULL).axf > $(APP_OUT_DIR)/$(TARGET_FULL).dis
	
###############################################################################

clean:
	rm -f $(APPOBJS) $(APPDEPS)
	rm -f $(APP_OUT_DIR)/$(TARGET_FULL).bin
	rm -f $(APP_OUT_DIR)/$(TARGET_FULL).axf
	rm -f $(APP_OUT_DIR)/$(TARGET_FULL).map
	rm -f $(APP_OUT_DIR)/$(TARGET_FULL).dis
	rm -f $(APP_OUT_DIR)/$(TARGET_FULL).log
	rm -f $(APP_OUT_DIR)/$(TARGET_FULL).txt
	rm -f $(APP_OUT_DIR)/$(TARGET_FULL).html

###############################################################################
