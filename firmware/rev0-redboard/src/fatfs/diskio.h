/*-----------------------------------------------------------------------
/  Low level disk interface modlue include file   (C)ChaN, 2012
/-----------------------------------------------------------------------*/

#ifndef _DISKIO_DEFINED
#define _DISKIO_DEFINED

#ifdef __cplusplus
extern "C" {
#endif

#define _USE_WRITE	1	/* 1: Enable disk_write function */
#define _READONLY	0	/* 1: Remove write functions */
#define _USE_IOCTL	1	/* 1: Enable disk_ioctl fucntion */

#define use_DMA_WRITE 	// Enable DMA mode in block write operations
#define use_DMA_READ 	// Enable DMA mode in block read operations

//Hardware Leverl----------------------------------------------------------------------
/* Port Controls  (Platform dependent) */
#define CS_SETOUT() TRISGbits.TRISG9 = 0
#define CS_LOW()  _LATG9 = 0	/* MMC CS = L */
#define CS_HIGH() _LATG9 = 1	/* MMC CS = H */

// Change the SPI port number as needed on the following 7 lines
#define SPIBRG  SPI2BRG
#define SPIBUF  SPI2BUF
#define SPISTATbits SPI2STATbits
#define SPI_CHANNEL SPI_CHANNEL2
#define SPICONbits SPI2CONbits
#define _SPI_TX_IRQ _SPI2_TX_IRQ
#define _SPI_RX_IRQ _SPI2_RX_IRQ
//-------------------------------------------------------------------------------------

#include "integer.h"


/* Status of Disk Functions */
typedef BYTE	DSTATUS;

/* Results of Disk Functions */
typedef enum {
	RES_OK = 0,		/* 0: Successful */
	RES_ERROR,		/* 1: R/W Error */
	RES_WRPRT,		/* 2: Write Protected */
	RES_NOTRDY,		/* 3: Not Ready */
	RES_PARERR		/* 4: Invalid Parameter */
} DRESULT;


/*---------------------------------------*/
/* Prototypes for disk control functions */


DSTATUS disk_initialize (BYTE);
DSTATUS disk_status (BYTE);
DRESULT disk_read (BYTE, BYTE*, DWORD, BYTE);
DRESULT disk_write (BYTE, const BYTE*, DWORD, BYTE);
DRESULT disk_ioctl (BYTE, BYTE, void*);

static BYTE send_cmd (BYTE cmd, DWORD arg);
#if _READONLY == 0
DRESULT disk_write (BYTE, const BYTE*, DWORD, BYTE);
static int xmit_datablock (const BYTE *buff,BYTE token);
#endif


/* Disk Status Bits (DSTATUS) */
#define STA_NOINIT		0x01	/* Drive not initialized */
#define STA_NODISK		0x02	/* No medium in the drive */
#define STA_PROTECT		0x04	/* Write protected */


/* Command code for disk_ioctrl fucntion */

/* Generic command (used by FatFs) */
#define CTRL_SYNC			0	/* Flush disk cache (for write functions) */
#define GET_SECTOR_COUNT	1	/* Get media size (for only f_mkfs()) */
#define GET_SECTOR_SIZE		2	/* Get sector size (for multiple sector size (_MAX_SS >= 1024)) */
#define GET_BLOCK_SIZE		3	/* Get erase block size (for only f_mkfs()) */
#define CTRL_ERASE_SECTOR	4	/* Force erased a block of sectors (for only _USE_ERASE) */

/* Generic command (not used by FatFs) */
#define CTRL_POWER			5	/* Get/Set power status */
#define CTRL_LOCK			6	/* Lock/Unlock media removal */
#define CTRL_EJECT			7	/* Eject media */
#define CTRL_FORMAT			8	/* Create physical format on the media */

/* MMC/SDC specific ioctl command */
#define MMC_GET_TYPE		10	/* Get card type */
#define MMC_GET_CSD			11	/* Get CSD */
#define MMC_GET_CID			12	/* Get CID */
#define MMC_GET_OCR			13	/* Get OCR */
#define MMC_GET_SDSTAT		14	/* Get SD status */

/* ATA/CF specific ioctl command */
#define ATA_GET_REV			20	/* Get F/W revision */
#define ATA_GET_MODEL		21	/* Get model name */
#define ATA_GET_SN			22	/* Get serial number */


/* MMC card type flags (MMC_GET_TYPE) */
#define CT_MMC		0x01		/* MMC ver 3 */
#define CT_SD1		0x02		/* SD ver 1 */
#define CT_SD2		0x04		/* SD ver 2 */
#define CT_SDC		(CT_SD1|CT_SD2)	/* SD */
#define CT_BLOCK	0x08		/* Block addressing */


#ifdef __cplusplus
}
#endif

#endif
