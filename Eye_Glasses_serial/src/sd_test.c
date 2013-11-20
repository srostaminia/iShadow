//#include <p32xxxx.h>
//#include <osc.h>
#include <plib.h>
//#include "sys_variables.h"
//#include "sys_config.h"
#include "sys_func.h"
#include ".\fatfs\ff.h"
#include ".\fatfs\diskio.h"


//Work registers for fs command
DWORD acc_size;			/* Work register for fs command */
WORD acc_files, acc_dirs;
FILINFO Finfo;
const BYTE ft[] = {0,12,16,32};

//File system object
FATFS Fatfs;
FATFS *fs;				/* Pointer to file system object */
FIL file1, file2;		/* File objects */

char speedBuffer[]= "IMTOTALLYGOINGSOFAST!!!!!!!!!!!!IMTOTALLYGOINGSOFAST!!!!!!!!!!!!IMTOTALLYGOINGSOFAST!!!!!!!!!!!!IMTOTALLYGOINGSOFAST!!!!!!!!!!!!IMTOTALLYGOINGSOFAST!!!!!!!!!!!!IMTOTALLYGOINGSOFAST!!!!!!!!!!!!IMTOTALLYGOINGSOFAST!!!!!!!!!!!!IMTOTALLYGOINGSOFAST!!!!!!!!!!!!IMTOTALLYGOINGSOFAST!!!!!!!!!!!!IMTOTALLYGOINGSOFAST!!!!!!!!!!!!IMTOTALLYGOINGSOFAST!!!!!!!!!!!!IMTOTALLYGOINGSOFAST!!!!!!!!!!!!IMTOTALLYGOINGSOFAST!!!!!!!!!!!!IMTOTALLYGOINGSOFAST!!!!!!!!!!!!IMTOTALLYGOINGSOFAST!!!!!!!!!!!!IMTOTALLYGOINGSOFAST!!!!!!!!!!!!";
char bigspeedBuffer[4096];


int sd_test (void)
{
//Mount Filesystem
f_mount(0, &Fatfs);
//Open/create Fileset.dex
FRESULT FOpenRes;
long accessmode;
long accessmode2;
char file1name[]= "Fileset.dex";
char file2name[]= "NBII000.dat";
accessmode2= (FA_CREATE_ALWAYS | FA_OPEN_ALWAYS | FA_CREATE_NEW | FA_WRITE |FA_READ);
accessmode= (FA_OPEN_ALWAYS | FA_WRITE |FA_READ);
FOpenRes=f_open(&file1, file1name, (BYTE)accessmode);


//Read 1 byte from file
char Buff=0;
unsigned int numread=0;

if(FOpenRes==FR_OK)
	{
		FOpenRes=f_read (&file1, &Buff, 1, &numread);
	}
else
	{
		//Create new file
		f_close(&file1);
		FOpenRes=f_open(&file1, file1name, (BYTE)accessmode2);
		numread=0;
	}

if (numread==0)
	{//no bytes read, new fileset
		Buff=0;
		FOpenRes=f_write(&file1, &Buff, 1, &numread);
		//numread recycled as numwritten
	}
	else
	{ //Overwrite first byte
		f_close(&file1);
		f_open(&file1,file1name, (BYTE)accessmode);
#ifdef TESTWRITE
		Buff++;
#endif
		FOpenRes=f_write(&file1, &Buff, 1, &numread);
	}

//close first file
f_close(&file1);


//Determine name of new file to create based on index number
if (Buff<=9)
	{file2name[6]=Buff+48;}
else if (Buff<=99)
	{file2name[5]=Buff/10 +48;
	 file2name[6]=Buff%10 +48;
	}

#ifdef TESTWRITE
//Recycle the pointer and the access mode
do{FOpenRes=f_open(&file1, file2name, (BYTE)accessmode2);}
while(FOpenRes!=FR_OK);

unsigned int loopdex8MB=0;
unsigned long num4kper8MB=2048;

//Fill large buffer
for(loopdex8MB=0; loopdex8MB<4096; loopdex8MB++)
	{bigspeedBuffer[loopdex8MB]=speedBuffer[loopdex8MB%32];}

//#define NUM8MB 118
#define NUM8MB 3

tick = 0;	// Reset benchmarking timer (to be read in debug mode)

for(loopdex8MB=0; loopdex8MB<num4kper8MB*NUM8MB; loopdex8MB++)
	{
		FOpenRes=f_write(&file1, bigspeedBuffer, 4096, &numread);
		if (FOpenRes)
		{
			Nop();	// Debug
			break;
		}
	}

//close second file
f_close(&file1);

//unmount filesystem
f_mount(0,NULL);
	
Nop();	// Read here the benchmarking timer
while(1);

#else

FOpenRes = f_open(&file1, file2name, FA_READ | FA_OPEN_EXISTING);
tick = 0;	// Reset benchmarking timer (to be read in debug mode)

while(1)
{
	FOpenRes = f_read(&file1, &bigspeedBuffer, sizeof(bigspeedBuffer), &numread);
	if (numread < sizeof(bigspeedBuffer) )
	{
		Nop(); 	// Debug
		break;
	}
}

//close second file
f_close(&file1);

//unmount filesystem
f_mount(0,NULL);
	
Nop();	// Read here the benchmarking timer
while(1);
#endif

}







