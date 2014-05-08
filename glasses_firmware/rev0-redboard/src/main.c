#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <plib.h>
#include "sys_variables.h"
#include "configuration_bits.h"
#include "sys_config.h"
#include "sys_func.h"
#include ".\fatfs\ff.h"
#include ".\fatfs\diskio.h"
#include "predict_gaze.h"

  // Result code
  FRESULT rc;
  //File system object
  FATFS fatfs;
  // File object
  FIL fil;
  UINT bw;

  // Predicted x and y gaze values
  extern unsigned short pred[2];
  //
  extern int start_signal;
  extern int num_frames;
  extern int file_set;

  extern unsigned short mask[NUM_SUBSAMPLE][2];
//************************************************************************************************
void main()
{

   //OSCCONSET = 0x10; // set Power-Saving mode to Sleep
   mPORTGSetPinsDigitalOut(BIT_15);
   sys_init();
    /*if ( RCON & 0x18 ) // The WDT caused a wake from Sleep
    {
        asm volatile("eret"); // return from interrupt
    }*/
   //set timage
   tick=0;

   //sleep();
   //WDTCONSET = 0x01;
   //asm volatile("wait");

   //PowerSaveSleep();
   //setInterruptAndSleep();

   //begin(SMH_VREF_3V3_outdoor,SMH_NBIAS_3V3_outdoor,SMH_AOBIAS_3V3_outdoor,SMH_GAIN_3V3_outdoor,SMH_SELAMP_3V3_outdoor);
   //begin(SMH_VREF_5V0,SMH_NBIAS_5V0,SMH_AOBIAS_5V0,SMH_GAIN_DEFAULT,SMH_SELAMP_DEFAULT);
   //Nbias???????0~63(???);
   //no lens
   //Nbias=60: Aobias????buffer: 0:????(0?????-20??)??????????. 63??????(1????-9000?????and??),????????
   //Nbias=5: Aobias: 0:????(0????and?????-0??)??????????. 63??????(0????-9000?????and??),????????
   
   //begin(60,63,50,1,0);
   //initialize stonyman
   #ifdef FAST_MODE
   begin(SMH_VREF_3V3,SMH_NBIAS_3V3,SMH_AOBIAS_3V3_F,SMH_GAIN_3V3_F,SMH_SELAMP_3V3);
   #endif

   #ifdef MEDIUM_MODE
   //begin(SMH_VREF_2V8,SMH_NBIAS_2V8,SMH_AOBIAS_2V8,SMH_GAIN_2V8,SMH_SELAMP_2V8);
   //begin(SMH_VREF_3V0,SMH_NBIAS_3V0,SMH_AOBIAS_3V0,SMH_GAIN_3V0,SMH_SELAMP_3V0);
   begin(SMH_VREF_3V3,SMH_NBIAS_3V3,SMH_AOBIAS_3V3_M,SMH_GAIN_3V3_M,SMH_SELAMP_3V3);
   //begin(SMH_VREF_4V0,SMH_NBIAS_4V0,SMH_AOBIAS_4V0,SMH_GAIN_4V0,SMH_SELAMP_4V0);
   //begin(SMH_VREF_5V0,SMH_NBIAS_5V0,SMH_AOBIAS_5V0,SMH_GAIN_DEFAULT,SMH_SELAMP_DEFAULT);
   #endif

   #ifdef QUALITY_MODE
   begin(SMH_VREF_3V3,SMH_NBIAS_3V3,SMH_AOBIAS_3V3,SMH_GAIN_3V3,SMH_SELAMP_3V3);
   #endif

   //singel pixel data capture
   /*
   f_mount(0, &fatfs);
   rc = f_open(&fil, "Move.dat", FA_WRITE | FA_CREATE_ALWAYS);
   if (rc)
   {
      Nop();	// Debug
   }
   //single_pixel(56,56);
   int aa=0;
   for(aa=0;aa<1000;aa++)
   {
       pixel_binning(0xFF,0xFF);
   }

   rc = f_close(&fil);
   f_mount(0,NULL);
   */

   //while(1)
   //{
       //single_pixel(56,56);
   //}

   //show command
   WriteString("Input '/start' to start sustained capture in a new file\r\n");
   WriteString("Input '/stop' to halt sustained capture");
   SMH_SetAllLow;

   num_frames=0;
   for(file_set=0;;)
   {
       /*while(start_signal!=1);
       file_set=simultaneous_capture(file_set);
       start_signal=0;*/

       /*if (start_signal == 1) {
           sustained_capture(file_set);
           file_set++;
       }*/
       start_signal = 1;
       //sustained_capture_no_save(0);
       //sustained_predict_sleep();
       //capture_and_predict();
   }

   //while (1);
   //mPORTGSetPinsDigitalOut(BIT_15);
   //mPORTGSetBits(BIT_15);
}

void setInterruptAndSleep()
{
    int start_time=0, current_time=0;//in the unit of min second(10^(-3)s)
    start_time=rtcHour*3600000+rtcMin*60000+rtcSec*1000+minSec;

    WDTCONCLR = 0x0002; // Disable WDT window mode
    WDTCONSET = 0x8000; // Enable the WDT
    WDTCONSET = 0x01;   // Clear the timer
    
    PowerSaveSleep();
    WDTCONCLR = 0x8000; // Disable the WDT
    current_time=rtcHour*3600000+rtcMin*60000+rtcSec*1000+minSec-start_time;
    start_time=rtcHour*3600000+rtcMin*60000+rtcSec*1000+minSec;
}

//get mask
void get_mask()
{
    char buffer1[100];
    char buffer2[100];
    char *filename;
    WriteString("Getting Mask...\r\n");
    sprintf(buffer1, "mask_%d", file_set);
    filename=buffer1;
    f_mount(0, &fatfs);
    // Open file
    rc = f_open(&fil, filename, FA_WRITE | FA_CREATE_ALWAYS);
    if (rc)
    {
        Nop();	// Debug
        WriteString("ERROR:Fail to get mask!\r\n");
    }
    chipToMatlab(0);
    f_write(&fil, val, 2*ROWS*COLS, &bw);/* The third parameter: Number of bytes(8 bits) to write */
    rc = f_close(&fil);
    f_mount(0,NULL);
    sprintf(buffer2, "Finsh getting Mask %s.\r\n", filename);
    WriteString(buffer2);
}

//serial output
//extern unsigned short val[ROWS*COLS];
void serial_out()
{
  char buffer[100];
  //while(1)
  //{
  chipToMatlab(0);
  
  WriteString("//********************************************************************************//\r\n\r\n");
  WriteString("Img = [\r\n");
  int i,j;
  for(i=0;i<112;i++)
  {
      for(j=0;j<112;j++)
      {
          sprintf(buffer, "%d  ", val[i*112+j]);
          WriteString(buffer);
          //WriteString(" ");
      }
      delay_one_tenth_us(10);
      WriteString(";\r\n");
  }
  WriteString("]\r\n");
  WriteString("//********************************************************************************//\r\n");
  //}
}

//simultaneous_capture with webcam
int simultaneous_capture(int file_name)
{
   char *filename;
   char *message;
   char buffer[100];
   int start_time=0, current_time=0;//in the unit of min second(10^(-3)s)

   // Register volume work area (never fails)
   f_mount(0, &fatfs);

   //check wether the number of frames to captured is inputed
   if(num_frames==0)
   {
       WriteString("Input '/frame+(integer)' to set the number of frames captured by stonyman(without '()')!\r\n");
       return;
   }
   //while(num_frames==0);

   sprintf(buffer, "%d", file_name);
   filename=buffer;

   
   // Open file
   rc = f_open(&fil, filename , FA_WRITE | FA_CREATE_ALWAYS);
   if (rc)
   {
       Nop();	// Debug
       WriteString("/!ERROR:Fail to creat stonyman data file!\r\n");
       start_signal=0;
       return;
   }

   int i;
   WriteString("/stonyman_started\r\n");
   start_time=rtcHour*3600000+rtcMin*60000+rtcSec*1000+minSec;
   for(i=0;i<num_frames;i++)
   {
       //record the time stamp of each
       current_time=rtcHour*3600000+rtcMin*60000+rtcSec*1000+minSec-start_time;
       sprintf(buffer, "/stonyman_timestamp+%d+%d\r\n", i, current_time);
       message=buffer;
       WriteString(message);
       //mPORTGToggleBits(BIT_15);
       chipToMatlab(i);
       //save file
       f_write(&fil, val, 2*ROWS*COLS, &bw);/* The third parameter: Number of bytes(8 bits) to write */
       //P4OUT &= ~BIT5;
       //__delay_cycles(100000 * SYS_CLK_MHZ);
   }
   rc = f_close(&fil);
   f_mount(0,NULL);
   WriteString("Stonyman Finish Capture\r\n");
   file_name++;
   sprintf(buffer, "/dir_set_r+%d\r\n", file_name);
   WriteString(buffer);
   start_signal=0;
   return file_name;
}

//simultaneous_capture with webcam
int sustained_capture(int file_name)
{
   char *filename;
   char *message;
   char buffer[100];
   int start_time=0, current_time=0;//in the unit of min second(10^(-3)s)

   // Register volume work area (never fails)
   f_mount(0, &fatfs);

   sprintf(buffer, "%d", file_name);
   filename=buffer;


   // Open file
   rc = f_open(&fil, filename , FA_WRITE | FA_CREATE_ALWAYS);
   if (rc)
   {
       Nop();	// Debug
       WriteString("/!ERROR:Fail to creat stonyman data file!\r\n");
       start_signal=0;
       return;
   }

   int i = 0;
   while (start_signal == 1)
   {
       start_time=rtcHour*3600000+rtcMin*60000+rtcSec*1000+minSec;
       //mPORTGToggleBits(BIT_15);
       chipToMatlab(i);
       //save file
       current_time=rtcHour*3600000+rtcMin*60000+rtcSec*1000+minSec-start_time;
       f_write(&fil, val, 2*ROWS*COLS, &bw);/* The third parameter: Number of bytes(8 bits) to write */
       //P4OUT &= ~BIT5;
       //__delay_cycles(100000 * SYS_CLK_MHZ);
       i++;
   }
   rc = f_close(&fil);
   f_mount(0,NULL);
   file_name++;
   WriteString("Capture halted\r\n");
   return file_name;
}

//simultaneous_capture with webcam
void single_transmit()
{
   chipToMatlab(0);

   int i, j;
   for (i = 0; i < ROWS; i++) {
       for (j = 0; j < COLS; j++)
       {
           //PutCharacter('a');
           PutCharacter(val[i*COLS + j]);
           PutCharacter(val[i*COLS + j] >> 8);
       }

       PutCharacter('\n');
   }
}

void capture_and_predict()
{
    int start_time=0, current_time=0;//in the unit of min second(10^(-3)s)
    start_time=rtcHour*3600000+rtcMin*60000+rtcSec*1000+minSec;
    subsample_capture(mask);
    predict_gaze();
    current_time=rtcHour*3600000+rtcMin*60000+rtcSec*1000+minSec-start_time;
    start_time=rtcHour*3600000+rtcMin*60000+rtcSec*1000+minSec;
}

void read_and_predict()
{
   FIL fil2;
   char *filename;
   char buffer[100];
   int start_time=0, current_time=0;//in the unit of min second(10^(-3)s)

   // Register volume work area (never fails)
   f_mount(0, &fatfs);

   sprintf(buffer, "%d", 0);
   filename=buffer;

   // Open file
   rc = f_open(&fil, filename , FA_READ);
   if (rc)
   {
       Nop();	// Debug
       WriteString("/!ERROR:Fail to creat stonyman data file!\r\n");
       return;
   }

   sprintf(buffer, "%d", 1);
   filename=buffer;

   // Open file
   rc = f_open(&fil2, filename , FA_WRITE | FA_CREATE_ALWAYS);
   if (rc)
   {
       Nop();	// Debug
       WriteString("/!ERROR:Fail to creat stonyman data file!\r\n");
       return;
   }

   WriteString("Beginning predictors...\r\n");

   int i;
   for (i = 0; i < 960; i++) {
        f_read(&fil, val, 2*NUM_SUBSAMPLE, &bw);
        start_time=rtcHour*3600000+rtcMin*60000+rtcSec*1000+minSec;
        predict_gaze();
        current_time=rtcHour*3600000+rtcMin*60000+rtcSec*1000+minSec-start_time;
        f_write(&fil2, pred, 4, &bw);
   }
   rc = f_close(&fil);
   rc = f_close(&fil2);
   f_mount(0,NULL);
   WriteString("Finished\r\n");
}

void capture_for_comparison()
{
   FIL fil2;
   char *filename;
   char buffer[100];

   // Register volume work area (never fails)
   f_mount(0, &fatfs);

   sprintf(buffer, "%d", 0);
   filename=buffer;

   // Open file
   rc = f_open(&fil, filename , FA_WRITE | FA_CREATE_ALWAYS);
   if (rc)
   {
       Nop();	// Debug
       WriteString("/!ERROR:Fail to creat stonyman data file!\r\n");
       return;
   }

   sprintf(buffer, "%d", 1);
   filename=buffer;

   // Open file
   rc = f_open(&fil2, filename , FA_WRITE | FA_CREATE_ALWAYS);
   if (rc)
   {
       Nop();	// Debug
       WriteString("/!ERROR:Fail to creat stonyman data file!\r\n");
       return;
   }

   subsample_capture(mask);
   subsample_capture(mask);
    f_write(&fil, val, 2*NUM_SUBSAMPLE, &bw);
    chipToMatlab(0);
    f_write(&fil2, val, 2*ROWS*COLS, &bw);
    
   rc = f_close(&fil);
   rc = f_close(&fil2);
   f_mount(0,NULL);
   WriteString("Finished\r\n");
}

//simultaneous_capture with webcam
int sustained_capture_no_save(int file_name)
{
   char *filename;
   char *message;
   char buffer[100];
   int start_time=0, current_time=0;//in the unit of min second(10^(-3)s)

   int i = 0;
   while (start_signal == 1)
   {
       //mPORTGToggleBits(BIT_15);
       chipToMatlab(i);
       //save file
       //P4OUT &= ~BIT5;
       //__delay_cycles(100000 * SYS_CLK_MHZ);
       i++;
   }
   return file_name;
}

void sustained_predict_sleep()
{
    while (start_signal == 1) {
        subsample_capture(mask);
        predict_gaze();
        setInterruptAndSleep();
    }
}