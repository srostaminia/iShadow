#include "TestSDCard.h"

//#include "ff.h"
#include "diskio.h"
#include "main.h"
#include "stm32l152d_eval_sdio_sd.h"
/* Code */

//#define NULL    0

int TestSDCard( )
{
  uint8_t buffer[512 * 7];//, buffer2[512 * 7];
//  uint32_t sector_adr, buffer_adr;
//  
//  uint8_t Counter = 1, sector = 0;
  
  for (int i = 0; i < 512 * 7; i++)
    buffer[i] = i;
  
//  buffer_adr = (uint32_t)(buffer + (Counter*512)); 
//  sector_adr = (sector << 9) + (512*Counter) ;
  
  if (disk_initialize(0) != SD_OK) {
    return -1;
  }
  else if (disk_write(0, (uint8_t *)buffer, 0, 7) != RES_OK) {
    return -2;
  }
//  else if (disk_read(0, (uint8_t *)buffer2, 0, 7) != RES_OK) {
//    return -3;
//  }
//  else if (buffer2[1] != 1) {
//    return -4;
//  }
  
//    if( f_open( &file, "test.dat", FA_CREATE_ALWAYS | FA_WRITE ) )
//    {
//        return -1;
//    }
//    else if( f_write( &file, buffer, 7, &writed ) )
//    {
//        return -1;
//    }
//    else if( f_close( &file ) )
//    {
//        return -1;
//    }
//    else if( f_mount( 0, NULL ) )
//    {
//        return -1;
//    }
    
    return 0;
}
