/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/


#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */

#include <Arduino.h>

#include "conf.h"
#include "SAMD_SPIFlash/src/Adafruit_SPIFlash.h"

/* Definitions of physical drive number for each drive */
#define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */

static Adafruit_FlashTransport_SPI _flashTransport(CHIP_SELECT, SPI);
static Adafruit_SPIFlash _flash(&_flashTransport);

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
  return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
  _flashTransport.setClockSpeed(MAX_FREQ, MAX_FREQ);
  if (_flash.begin()) return RES_OK;
  else return RES_ERROR;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
  BYTE * b = buff;
  if(_flash.readBlocks(sector, b, count)) return RES_OK;
  else return RES_ERROR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
  BYTE * b = (BYTE*) buff;
  if (_flash.writeBlocks(sector, buff, count)) return RES_OK;
  else return RES_ERROR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
  DRESULT res = RES_ERROR;  
  
  uint8_t id[5];
  switch( cmd )
  {
    case CTRL_SYNC :   // Make sure that data has been written  
      _flash.syncBlocks();
      res = RES_OK;
      break;
    
    case GET_SECTOR_COUNT : 
      *(LBA_t*)buff = _flash.size() / SECTOR_SIZE;
      res = RES_OK;
      break;  

    case GET_SECTOR_SIZE :
      *(WORD*)buff = SECTOR_SIZE;
      res = RES_OK;
      break;  

    case GET_BLOCK_SIZE :
      *(DWORD*)buff = BLOCK_SIZE;
      res = RES_OK;
      break;  
	  
    default:  
      res = RES_PARERR;  
  }

  return res;
}
