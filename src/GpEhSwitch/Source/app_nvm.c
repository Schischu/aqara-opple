/****************************************************************************
 *
 * MODULE:             JN-AN-1245
 *
 * COMPONENT:          AHI_EEPROM.c
 *
 * DESCRIPTION:
 * Basic API for EEPROM access on JN516x
 *
 ****************************************************************************
 *
 * This software is owned by NXP B.V. and/or its supplier and is protected
 * under applicable copyright laws. All rights are reserved. We grant You,
 * and any third parties, a license to use this software solely and
 * exclusively on NXP products [NXP Microcontrollers such as JN5168, JN5169,
 * JN5179, JN5189].
 * You, and any third parties must reproduce the copyright and warranty notice
 * and any other legend of ownership on each copy or partial copy of the
 * software.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Copyright NXP B.V. 2016-2018. All rights reserved
 *
 ***************************************************************************/

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include "jendefs.h"
#include "fsl_flash.h"
#include "APP_nvm.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/


/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
#define  NVM_BYTES_PER_SEGMENT   512
#define  NVM_MAGIC_NO            0xA8
#define  NVM_START_SECTOR        700
#define  NVM_USE_MAX_SECTORS     14
/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
static uint8  au8GPDataBuffer [NVM_BYTES_PER_SEGMENT];
static uint32 u32CurrentPage =  NVM_START_SECTOR;
/****************************************************************************/
/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
/****************************************************************************/

/****************************************************************************
 **
 ** NAME:      bAPP_ReadBlock
 **
 ** DESCRIPTION:
 ** Read bytes from the Flash into a user-supplied buffer.
 **
 ** PARAMETERS: Name          Usage
 ** uint16      u16StartAddr  Start address in flash
 ** uint16      u16Bytes      Bytes to read ( max 512 bytes )
 ** uint8 *     pu8Buffer     Buffer for read data
 **
 ** RETURN:
 ** TRUE if success, FALSE if parameter error
 **
 ****************************************************************************/
PUBLIC bool_t bAPP_ReadBlock(      uint16  u16Bytes,
                                   uint8  *pu8Buffer )
{
    int               i;
    int               j;
    volatile uint8    *pu8FlashAddress;
    int               iAPIreturn;

    for (i = 0; i < NVM_USE_MAX_SECTORS; i++)
    {
        // calculate FLASH start address
        pu8FlashAddress = (uint8 *)( ( ( u32CurrentPage + i ) * NVM_BYTES_PER_SEGMENT) );
        iAPIreturn =
        FLASH_BlankCheck( FLASH,
                         (uint8 *)( pu8FlashAddress ),
                         (uint8 *)( ( pu8FlashAddress ) + (NVM_BYTES_PER_SEGMENT - 1 ) ) );
        if ( iAPIreturn != FLASH_DONE )
        {
            for( j = 0 ; j < u16Bytes; j++ )
            {
                //loop on bytes
                au8GPDataBuffer [ j ] = pu8FlashAddress [ j ];
            }
            memcpy ( pu8Buffer, au8GPDataBuffer, sizeof( tGPD_PersistantData ) );
            if( ( ( tGPD_PersistantData* )( pu8Buffer ) )->u8DataValid == NVM_MAGIC_NO )
            {
                u32CurrentPage = u32CurrentPage + i;
                return TRUE;
            }
        }
    }
    return TRUE;

}


/****************************************************************************
 **
 ** NAME:      bAPP_NVMWriteBlock
 **
 ** DESCRIPTION:
 ** Write bytes to the EEPROM. Writes can span across segments.
 **
 ** PARAMETERS: Name       Usage
 ** uint16      u16StartAddr  Start address in flash
 ** uint16      u16Bytes      Bytes to write ( max 512 bytes )
 ** uint8 *     pu8Buffer     Buffer for write data
 **
 ** RETURN:
 ** TRUE if success, FALSE if parameter error
 **
 ****************************************************************************/
PUBLIC bool_t bAPP_NVMWriteBlock(   uint16  u16Bytes,
                                    uint8  *pu8Buffer )
{

    if (u16Bytes > NVM_BYTES_PER_SEGMENT)
        u16Bytes = NVM_BYTES_PER_SEGMENT;
    memcpy(au8GPDataBuffer, pu8Buffer,u16Bytes);
    u32CurrentPage = u32CurrentPage+1;
    return bAPP_NVMWrite(u32CurrentPage, 0, u16Bytes, au8GPDataBuffer );
}

/****************************************************************************
 **
 ** NAME:      bAPP_NVMWrite
 **
 ** DESCRIPTION:
 ** Write bytes to the specified segment and offset within that segment. Only
 ** writes within one segment per call are allowed.
 **
 ** PARAMETERS: Name       Usage
 ** uint8       u8Segment  flash Segment Number
 ** uint8       u8Offset   Byte offset into segment
 ** uint8       u8Bytes    Bytes to write
 ** uint8 *     pu8Buffer  Buffer for write data
 **
 ** RETURN:
 ** TRUE if success, FALSE if parameter error
 **
 ****************************************************************************/
PUBLIC bool_t bAPP_NVMWrite(   uint32  u32Segment,
                               uint8   u8Offset,
                               uint8   u8Bytes,
                               uint8   *pu8Buffer)
{
    int iAPIreturn;
    // Erase Current Data In Specified Page

    if (u32Segment > (NVM_START_SECTOR + NVM_USE_MAX_SECTORS) )
        u32Segment = NVM_START_SECTOR;

    iAPIreturn =  FLASH_ErasePages(FLASH, u32Segment, 1);
    if(iAPIreturn == FLASH_DONE)
    {
        iAPIreturn =
        FLASH_BlankCheck( FLASH,
                          (uint8 *)( u32Segment * NVM_BYTES_PER_SEGMENT ),
                          (uint8 *)( ( u32Segment * NVM_BYTES_PER_SEGMENT ) + ( NVM_BYTES_PER_SEGMENT- 1 ) ) );
        if (iAPIreturn == FLASH_DONE)
        {
            // program data
            tGPD_PersistantData *psStruct = (tGPD_PersistantData *)pu8Buffer;
            psStruct->u8DataValid = NVM_MAGIC_NO;
            iAPIreturn = FLASH_Program( FLASH,
                                        (uint32 *)(u32Segment * NVM_BYTES_PER_SEGMENT),
                                        (uint32*)psStruct,
                                        NVM_BYTES_PER_SEGMENT );
            (void) FLASH_ErasePages(FLASH, (u32Segment-1), 1);
        }
    }

    if (iAPIreturn == FLASH_DONE)
    {
        return TRUE;
    }

    return FALSE;

}


/****************************************************************************/
/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/
/****************************************************************************/

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
