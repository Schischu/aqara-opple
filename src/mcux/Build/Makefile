############################################################################
#
# MODULE              JN-AN-1245
#
# DESCRIPTION         All Build Configurations Makefile
#
############################################################################
#
# This software is owned by NXP B.V. and/or its supplier and is protected
# under applicable copyright laws. All rights are reserved. We grant You,
# and any third parties, a license to use this software solely and
# exclusively on NXP products [NXP Microcontrollers such as JN5168, JN5164]. 
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
# Copyright NXP B.V. 2016-2018. All rights reserved
#
############################################################################
# Build each target in turn
all:
	rm -f ../../Doc/size.txt
	$(MAKE) -C ../../Common_Controller/Build/mcux REMOTE=ColorSceneController DR=OM15082 TRACE=1 APP_NTAG_ICODE=0 OTA=0 RAMOPT=0 clean -r
	$(MAKE) -C ../../Common_Controller/Build/mcux REMOTE=ColorSceneController DR=OM15082 TRACE=1 APP_NTAG_ICODE=0 OTA=0 RAMOPT=0 all -r
	$(MAKE) -C ../../Common_Controller/Build/mcux REMOTE=ColorSceneController DR=OM15082 TRACE=1 APP_NTAG_ICODE=1 OTA=1 RAMOPT=1 clean -r
	$(MAKE) -C ../../Common_Controller/Build/mcux REMOTE=ColorSceneController DR=OM15082 TRACE=1 APP_NTAG_ICODE=1 OTA=1 RAMOPT=1 all -r
	$(MAKE) -C ../../Common_Switch/Build/mcux SWITCH=DimmerSwitch DR=OM15082 TRACE=1 APP_NTAG_ICODE=0 OTA=0 RAMOPT=0 clean -r
	$(MAKE) -C ../../Common_Switch/Build/mcux SWITCH=DimmerSwitch DR=OM15082 TRACE=1 APP_NTAG_ICODE=0 OTA=0 RAMOPT=0 all -r
	$(MAKE) -C ../../Common_Switch/Build/mcux SWITCH=DimmerSwitch DR=OM15082 TRACE=1 APP_NTAG_ICODE=1 OTA=1 RAMOPT=1 clean -r
	$(MAKE) -C ../../Common_Switch/Build/mcux SWITCH=DimmerSwitch DR=OM15082 TRACE=1 APP_NTAG_ICODE=1 OTA=1 RAMOPT=1 all -r
	$(MAKE) -C ../../GpEhSwitch/Build/mcux clean -r
	$(MAKE) -C ../../GpEhSwitch/Build/mcux all -r
	
