/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/

#include "diskio.h"
#include "stm32l1xx.h"
#include "ffconf.h"
#include "stm32l152d_eval_sdio_sd.h"

extern uint32_t sd_ptr;

/*-----------------------------------------------------------------------*/
/* Correspondence between physical drive number and physical drive.      */
/*-----------------------------------------------------------------------*/

#define AT    0
#define MMC   1
#define USB   2
SD_CardInfo SDCardInfo2;

// Pointer to next location to write in SD card
uint32_t sd_ptr;

/*----------------------------------------------------------------------------*/
/* Initialize a Drive                                                         */
/*----------------------------------------------------------------------------*/

DSTATUS disk_initialize (
  BYTE drv    /* Physical drive nmuber (0..) */
)
{
  sd_ptr = 0;
  SD_Error res = SD_ERROR;
  res =  SD_Init(); 
  return ((DSTATUS)res);
}

/*----------------------------------------------------------------------------*/
/* Return Disk Status                                                         */
/*----------------------------------------------------------------------------*/

DSTATUS disk_status (
  BYTE drv    /* Physical drive nmuber (0) */
)
{
  if (drv) return STA_NOINIT;    /* Supports only single drive */
  return 0;
}

/*----------------------------------------------------------------------------*/
/* Read Sector(s)                                                             */
/*----------------------------------------------------------------------------*/

DRESULT disk_read (
  BYTE drv,     /* Physical drive nmuber (0..) */
  BYTE *buff,   /* Data buffer to store read data */
  DWORD sector, /* Sector address (LBA) */
  BYTE count    /* Number of sectors to read (1..255) */
)
{
  SD_ReadBlock(buff, sector << 9, 512);
  while(SD_GetStatus() != SD_TRANSFER_OK)
  {
  }
  return RES_OK;
}

#if _READONLY == 0
DRESULT disk_write_ptr(
  BYTE drv,          /* Physical drive nmuber (0..) */
  const BYTE *buff,  /* Data to be written */
  BYTE count         /* Number of sectors to write (1..255) */
)
{
  DRESULT status = disk_write(drv, buff, sd_ptr, count);
  
  sd_ptr += count;
  
  return status;
}
#endif /* _READONLY */

/*----------------------------------------------------------------------------*/
/* Write Sector(s)                                                            */
/*----------------------------------------------------------------------------*/
/* The FatFs module will issue multiple sector transfer request
/  (count > 1) to the disk I/O layer. The disk function should process
/  the multiple sector transfer properly Do. not translate it into
/  multiple single sector transfers to the media, or the data read/write
/  performance may be drasticaly decreased. */

#if _READONLY == 0
DRESULT disk_write (
  BYTE drv,          /* Physical drive nmuber (0..) */
  const BYTE *buff,  /* Data to be written */
  DWORD sector,      /* Sector address (LBA) */
  BYTE count         /* Number of sectors to write (1..255) */
)
{
  uint32_t Counter = 0;
  uint32_t buffer_adr =0 , sector_adr = 0;
  
  for(Counter = 0; Counter < count; Counter++ )
  {
    buffer_adr = (uint32_t)(buff + (Counter*512));
    sector_adr = (sector << 9) + (512*Counter) ;
    SD_WriteMultiBlocks((BYTE *)(buffer_adr), sector_adr , 512, 1); 
    SD_WaitWriteOperation();
    while(SD_GetStatus() != SD_TRANSFER_OK)
    {
    }
  }
  return RES_OK;
}
#endif /* _READONLY */

/*----------------------------------------------------------------------------*/
/* Write Sector(s)                                                            */
/*----------------------------------------------------------------------------*/
/* The FatFs module will issue multiple sector transfer request
/  (count > 1) to the disk I/O layer. The disk function should process
/  the multiple sector transfer properly Do. not translate it into
/  multiple single sector transfers to the media, or the data read/write
/  performance may be drasticaly decreased. */

#if _READONLY == 0
DRESULT disk_write_fast (
  BYTE drv,          /* Physical drive nmuber (0..) */
  const BYTE *buff,  /* Data to be written */
  DWORD sector,      /* Sector address (LBA) */
  BYTE count         /* Number of sectors to write (1..255) */
)
{
//  uint32_t Counter = 0;
  uint32_t buffer_adr =0 , sector_adr = 0;
  
  buffer_adr = (uint32_t)(buff);
  sector_adr = (sector << 9) ;
  SD_WriteMultiBlocks((BYTE *)(buffer_adr), sector_adr , 512, count);
//  SD_WaitWriteOperation();
//  while(SD_GetStatus() != SD_TRANSFER_OK);
  
  return RES_OK;
}

void f_finish_write()
{
  SD_WaitWriteOperation();
  while(SD_GetStatus() != SD_TRANSFER_OK);
}

DRESULT disk_erase (
  BYTE drv,          /* Physical drive nmuber (0..) */
  DWORD sector,      /* Sector address (LBA) */
  BYTE count         /* Number of sectors to write (1..255) */
)
{
  uint32_t sector_start, sector_end;
  sector_start = (sector << 9) ;
  sector_end = ((sector + (count - 1)) << 9);
  
  if (SD_Erase(sector_start, sector_end) == SD_OK)
    return RES_OK;
  else
    return RES_ERROR;
}
#endif /* _READONLY */

/*----------------------------------------------------------------------------*/
/* Get current time                                                           */
/*----------------------------------------------------------------------------*/

DWORD get_fattime ()
{
  return  ((2006UL-1980) << 25)      /* Year = 2006 */
           | (2UL << 21)             /* Month = Feb */
           | (9UL << 16)             /* Day = 9 */
           | (22U << 11)             /* Hour = 22 */
           | (30U << 5)              /* Min = 30 */
           | (0U >> 1)               /* Sec = 0 */
           ;
}

/*----------------------------------------------------------------------------*/
/* Miscellaneous Functions                                                    */
/*----------------------------------------------------------------------------*/

DRESULT disk_ioctl (
  BYTE drv,    /* Physical drive nmuber (0..) */
  BYTE ctrl,   /* Control code */
  void *buff   /* Buffer to send/receive control data */
)
{
  DRESULT res = RES_OK;
  switch (ctrl) {

  case GET_SECTOR_COUNT :  /* Get number of sectors on the disk (DWORD) */
    *(DWORD*)buff = 131072;  /* 4*1024*32 = 131072 */
    res = RES_OK;
    break;

  case GET_SECTOR_SIZE :  /* Get R/W sector size (WORD) */
    *(WORD*)buff = 512;
    res = RES_OK;
    break;

  case GET_BLOCK_SIZE :  /* Get erase block size in unit of sector (DWORD) */
    *(DWORD*)buff = 32;
    res = RES_OK;
  }
  return res;
}
