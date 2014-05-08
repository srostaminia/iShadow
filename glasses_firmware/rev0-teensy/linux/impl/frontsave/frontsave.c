//**************************************************************************************************
// frontsave.c
//
// Russ Bielawski (edited down by Addison Mayberry from frontdisplayandsave.c)
// 2012-10-29: created as captureStreamDual.c
// 2012-11-12: split out front end display and save functionality
//**************************************************************************************************


//**************************************************************************************************
// includes
//
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <assert.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

// opencv sources
#include "cv.h"
#include "highgui.h"

#include "glasses.h"


//**************************************************************************************************
// defines / constants
//
#define NS_PER_SEC              (1000*1000*1000)
#define MAX_CAMS                (2)
#define FRAME_X_Y               (112)
#define FRAME_LEN               (FRAME_X_Y*FRAME_X_Y)
#define SCALINGVAL              (4)
#define KEY_ESC                 (27)
#define KEY_QUIT                ('q')
#define OUTPATH_MAX_LEN         (PATH_MAX_LEN)


//**************************************************************************************************
// globals
//
static FILE     *gOutfilefps;
static char     gOutpath[OUTPATH_MAX_LEN];
static unsigned gFlagUserCliValid;
static unsigned gFlagUserCliHelp;
static unsigned gFlagUserCliModeSelected;
static unsigned gFlagNoWriteVideo;
static unsigned gFlagRawMode;
static unsigned gFlagDrawGrid;


//**************************************************************************************************
// local function prototypes
//
static void printusage(char *progname);
static void printhelp(char *progname);
static int  parseargs(int argc, char **argv);
static void terminate(int xx);


//**************************************************************************************************
// main
//
int main(int argc, char** argv)
{
   int ii,jj,xx,yy;
   char cc;
   char indat[256*1024];    // huge because I am a lazy man
   char *indatloc;
   int readcnt;
   int totallen;

   unsigned numcams;

   IplImage *frame, *framenorm, *framescaledup;
   uchar *frameloc, *framenormloc, *framescaleduploc;
   uchar framevalmin, framevalmax;
   IplImage *frame2, *frame2norm, *frame2scaledup;
   uchar *frame2loc, *frame2normloc, *frame2scaleduploc;
   uchar frame2valmin, frame2valmax;

   // double wide!
   IplImage *framedualnorm, *framedualscaledup;
   uchar *framedualnormloc1, *framedualnormloc2, *framedualscaleduploc1, *framedualscaleduploc2;

   IplImage *gridoverlay;

   struct timespec time, timeprevious;
   double fpsinstant;
   double fpsmin;
   double fpsmax;

   struct stat outpathst = {0};
   char outfilenameprefix[2*OUTPATH_MAX_LEN];
   char outfilenameframe[2*OUTPATH_MAX_LEN];
   char outfilenameframe2[2*OUTPATH_MAX_LEN];
   unsigned int frameidx;

   /* TODO russ: can't get writing video to work yet!
   CvVideoWriter *vidout;*/


   // appease the compiler
   gOutfilefps = 0;


   // process user cli
   gFlagUserCliValid=0;
   gFlagUserCliHelp=0;
   gFlagUserCliModeSelected=0;
   gFlagNoWriteVideo=0;
   gFlagRawMode=0;
   gFlagDrawGrid=0;
   if(0 != parseargs(argc,argv))
   {
      printusage(argv[0]);
      exit(1);
   }
   if(0 == gFlagUserCliValid)
   {
      printusage(argv[0]);
      exit(1);
   }
   if(0 != gFlagUserCliHelp)
   {
      printhelp(argv[0]);
      exit(0);
   }
   if(0 == gFlagNoWriteVideo)
   {
      if(0 == stat(gOutpath,&outpathst))
      {
         fprintf(stderr,"ERROR: file %s already exists!\n",gOutpath);
         printusage(argv[0]);
         exit(1);
      }
   }


   if(0 != gFlagNoWriteVideo)
   {
      printf("quiet mode: video will not be saved\n");
   }
   else
   {
      getdeepestdirname(gOutpath,outfilenameprefix);
   }

   frameidx=0;


   // init our frame
   frame = cvCreateImage(cvSize(FRAME_X_Y,FRAME_X_Y), IPL_DEPTH_8U, 1);
   framenorm = cvCreateImage(cvSize(FRAME_X_Y,FRAME_X_Y), IPL_DEPTH_8U, 1);
   framescaledup = cvCreateImage(cvSize( FRAME_X_Y*SCALINGVAL,
                                         FRAME_X_Y*SCALINGVAL ), IPL_DEPTH_8U, 1);
   frame2 = cvCreateImage(cvSize(FRAME_X_Y,FRAME_X_Y), IPL_DEPTH_8U, 1);
   frame2norm = cvCreateImage(cvSize(FRAME_X_Y,FRAME_X_Y), IPL_DEPTH_8U, 1);
   frame2scaledup = cvCreateImage(cvSize( FRAME_X_Y*SCALINGVAL,
                                          FRAME_X_Y*SCALINGVAL ), IPL_DEPTH_8U, 1);
   framedualnorm = cvCreateImage(cvSize(FRAME_X_Y*2,FRAME_X_Y), IPL_DEPTH_8U, 1);
   framedualscaledup = cvCreateImage(cvSize( FRAME_X_Y*SCALINGVAL*2,
                                             FRAME_X_Y*SCALINGVAL ), IPL_DEPTH_8U, 1);
   gridoverlay = cvCreateImage(cvSize( FRAME_X_Y*SCALINGVAL*2,
                                       FRAME_X_Y*SCALINGVAL ), IPL_DEPTH_8U, 3);
   // appease the compiler
   frame2loc = frame2normloc = frame2scaleduploc = 0;
   framedualnormloc1 = framedualnormloc2 = framedualscaleduploc1 = framedualscaleduploc2 = 0;


   time.tv_sec = time.tv_nsec = timeprevious.tv_sec = timeprevious.tv_nsec = 0;
   fpsinstant = fpsmin = fpsmax = -1;


   readuntilchar(stdin,SYMBOL_SOF);
   indat[0] = readchar(stdin);
   assert(OPCODE_RESP_NUM_CAMS == (unsigned char)indat[0]);
   numcams = (unsigned)readchar(stdin);
   assert((0 < numcams) && (MAX_CAMS >= numcams));

	readuntilchar(stdin,SYMBOL_SOF);
	indat[0] = readchar(stdin);
	assert( (OPCODE_FRAME == (unsigned char)indat[0]) ||
	      (SYMBOL_EXIT  == (unsigned char)indat[0]) );
	if(SYMBOL_EXIT == (unsigned char)indat[0])
	{
	 printf("Error: received exit instead of frame\n");
         return 0;
	}

	totallen=0;
	indatloc=indat;
	while(FRAME_LEN*numcams > totallen)
	{
	 readcnt = fread(indatloc,1,(FRAME_LEN*numcams)-totallen,stdin);
	 totallen+=readcnt;
	 indatloc+=readcnt;
	}
	*indatloc = '\0';


	// find max and min pixel values for normalization
	// TODO: should be a function?
	framevalmin = 255;
	framevalmax = 0;
	frame2valmin = 255;
	frame2valmax = 0;
	for(ii = 0; ii < FRAME_X_Y; ++ii)
	{
	 frameloc = (uchar*)(frame->imageData + (ii*frame->widthStep));
	 if(2 == numcams)
	 {
	    frame2loc = (uchar*)(frame2->imageData + (ii*frame2->widthStep));
	 }
	 for(jj = 0; jj < FRAME_X_Y; ++jj)
	 {
	    if(framevalmin > (unsigned char)indat[((numcams*ii)*FRAME_X_Y)+jj])
	    {
	       framevalmin = (unsigned char)indat[((numcams*ii)*FRAME_X_Y)+jj];
	    }
	    if(framevalmax < (unsigned char)indat[((numcams*ii)*FRAME_X_Y)+jj])
	    {
	       framevalmax = (unsigned char)indat[((numcams*ii)*FRAME_X_Y)+jj];
	    }
	    frameloc[jj] = (unsigned char)indat[((numcams*ii)*FRAME_X_Y)+jj];
	    if(2 == numcams)
	    {
	       if(frame2valmin > (unsigned char)indat[((numcams*ii+1)*FRAME_X_Y)+jj])
	       {
		  frame2valmin = (unsigned char)indat[((numcams*ii+1)*FRAME_X_Y)+jj];
	       }
	       if(frame2valmax < (unsigned char)indat[((numcams*ii+1)*FRAME_X_Y)+jj])
	       {
		  frame2valmax = (unsigned char)indat[((numcams*ii+1)*FRAME_X_Y)+jj];
	       }
	       frame2loc[jj] = (unsigned char)indat[((numcams*ii+1)*FRAME_X_Y)+jj];
	    }
	 }
	}

	// normalize and scale-up for display on screen
	// russ: this is pretty much a total hack!
	if(0 != gFlagRawMode)
	{
	 framevalmin = 0;
	 framevalmax = 255;
	 if(2 == numcams)
	 {
	    frame2valmin = 0;
	    frame2valmax = 255;
	 }
	}
	// TODO: should be function(s)?
	for(ii = 0; ii < FRAME_X_Y; ++ii)
	{
	 frameloc = (uchar*)(frame->imageData + (ii*frame->widthStep));
	 framenormloc = (uchar*)(framenorm->imageData + (ii*framenorm->widthStep));
	 if(2 == numcams)
	 {
	    frame2loc = (uchar*)(frame2->imageData + (ii*frame2->widthStep));
	    frame2normloc = (uchar*)(frame2norm->imageData + (ii*frame2norm->widthStep));
	    framedualnormloc1 = (uchar*)(framedualnorm->imageData + (ii*framedualnorm->widthStep));
	    framedualnormloc2 = (uchar*)( framedualnorm->imageData
		                          + (ii*framedualnorm->widthStep)
		                          + (framedualnorm->widthStep/2) );
	 }
	 for(jj = 0; jj < FRAME_X_Y; ++jj)
	 {
	    framenormloc[jj] = (uchar)((frameloc[jj]-framevalmin)*(255.0/framevalmax));
	    if(2 == numcams)
	    {
	       frame2normloc[jj] = (uchar)((frame2loc[jj]-frame2valmin)*(255.0/frame2valmax));
	       framedualnormloc1[jj] = (uchar)((frameloc[jj]-framevalmin)*(255.0/framevalmax));
	       framedualnormloc2[jj] = (uchar)((frame2loc[jj]-frame2valmin)*(255.0/frame2valmax));
	    }

	    for(xx=0; xx<SCALINGVAL; ++xx)
	    {
	       framescaleduploc
		  = (uchar*)( framescaledup->imageData
		              + (((ii*SCALINGVAL)+xx)*framescaledup->widthStep) );
	       if(2 == numcams)
	       {
		  frame2scaleduploc
		     = (uchar*)( frame2scaledup->imageData
		                 + (((ii*SCALINGVAL)+xx)*frame2scaledup->widthStep) );
		  // double wide
		  framedualscaleduploc1
		     = (uchar*)( framedualscaledup->imageData
		                 + (((ii*SCALINGVAL)+xx)*framedualscaledup->widthStep) );
		  framedualscaleduploc2 
		     = (uchar*)( framedualscaledup->imageData
		                 + (((ii*SCALINGVAL)+xx)*framedualscaledup->widthStep)
		                 + (framedualscaledup->widthStep/2) );
	       }
	       for(yy=0; yy<SCALINGVAL; ++yy)
	       {
		  framescaleduploc[(jj*SCALINGVAL)+yy]
		     = (uchar)((frameloc[jj]-framevalmin)*(255.0/framevalmax));
		  if(2 == numcams)
		  {
		     frame2scaleduploc[(jj*SCALINGVAL)+yy]
		        = (uchar)((frame2loc[jj]-frame2valmin)*(255.0/frame2valmax));
		     // double wide
		     framedualscaleduploc1[(jj*SCALINGVAL)+yy]
		        = (uchar)((frameloc[jj]-framevalmin)*(255.0/framevalmax));
		     framedualscaleduploc2[(jj*SCALINGVAL)+yy]
		        = (uchar)((frame2loc[jj]-frame2valmin)*(255.0/frame2valmax));
		  }
	       }
	    }
	 }
	}



	if(0 != gFlagDrawGrid)
	{
	 // FIXME only works with 2 cameras!
	 assert(2 == numcams);
	 cvCvtColor(framedualscaledup, gridoverlay, CV_GRAY2RGB);
	 cvLine( gridoverlay,
		 cvPoint((0+FRAME_X_Y)*SCALINGVAL,(FRAME_X_Y/3)*SCALINGVAL),
		 cvPoint((FRAME_X_Y+FRAME_X_Y)*SCALINGVAL,(FRAME_X_Y/3)*SCALINGVAL),
		 CV_RGB(255,0,0),1,8,0 );
	 cvLine( gridoverlay,
		 cvPoint((0+FRAME_X_Y)*SCALINGVAL,(2*FRAME_X_Y/3)*SCALINGVAL),
		 cvPoint((FRAME_X_Y+FRAME_X_Y)*SCALINGVAL,(2*FRAME_X_Y/3)*SCALINGVAL),
		 CV_RGB(255,0,0),1,8,0 );
	 cvLine( gridoverlay,
		 cvPoint((FRAME_X_Y/3+FRAME_X_Y)*SCALINGVAL,0*SCALINGVAL),
		 cvPoint((FRAME_X_Y/3+FRAME_X_Y)*SCALINGVAL,FRAME_X_Y*SCALINGVAL),
		 CV_RGB(255,0,0),1,8,0 );
	 cvLine( gridoverlay,
		 cvPoint((2*FRAME_X_Y/3+FRAME_X_Y)*SCALINGVAL,0*SCALINGVAL),
		 cvPoint((2*FRAME_X_Y/3+FRAME_X_Y)*SCALINGVAL,FRAME_X_Y*SCALINGVAL),
		 CV_RGB(255,0,0),1,8,0 );
	 cvShowImage("CamCapDoubleWide", gridoverlay);
	} else
      {
         // display picture on screen
         if(2 == numcams)
         {
            cvShowImage("CamCapDoubleWideSmall", framedualnorm);
         }
         else
         {
            cvShowImage("CamCapSmall", framenorm);
         }
      }

	// save the frame as a BMP file
	// TODO: should be a function?
	// TODO: save as video instead
	if(0 == gFlagNoWriteVideo)
	{
	 snprintf( outfilenameframe,2*OUTPATH_MAX_LEN,"%s_eye.bmp",outfilenameprefix);
	 snprintf( outfilenameframe2,2*OUTPATH_MAX_LEN,"%s_out.bmp",outfilenameprefix);

	 if(2 == numcams)
	 {
	    (void)cvSaveImage(outfilenameframe,framenorm,0);
	    (void)cvSaveImage(outfilenameframe2,frame2norm,0);
	 }
	 else
	 {
	    (void)cvSaveImage(outfilenameframe,framenorm,0);
	 }
	}

   // release/destroy OpenCV objects
   cvReleaseImage(&frame);
   cvReleaseImage(&framenorm);
   cvReleaseImage(&framescaledup);
   cvReleaseImage(&frame2);
   cvReleaseImage(&frame2norm);
   cvReleaseImage(&frame2scaledup);
   cvReleaseImage(&framedualnorm);
   cvReleaseImage(&framedualscaledup);
   if(2 == numcams)
   {
      cvDestroyWindow("CamCapDoubleWide");
      cvDestroyWindow("CamCapDoubleWideSmall");
   }
   else
   {
      cvDestroyWindow("CamCap");
      cvDestroyWindow("CamCapSmall");
   }

   return 0;
}


//**************************************************************************************************
// local function definitions
//

//
// printusage: prints a usage string for the program
//
static void printusage(char *progname)
{

   fprintf(stderr, "Usage: %s [-o PATH|-q]\n", progname);
}

//
// printhelp: prints the help for the program
//
static void printhelp(char *progname)
{
   printusage(progname);
   fprintf(stderr,"TODO: help not well written\n");
   fprintf(stderr,"press ESC to end the program (user must have context of the video window!).\n");
   fprintf(stderr,"\n");
   fprintf(stderr,"quick and dirty argument descriptions:\n");
   fprintf(stderr,"  -g         draw 3x3 grid lines\n");
   fprintf(stderr,"  -h         show help and exit\n");
   fprintf(stderr,"  -o PATH    save video to PATH.  PATH cannot already exist.  incompatible with -q\n");
   fprintf(stderr,"  -q         quiet mode.  won't write out any video.  incompatible with -o\n");
   fprintf(stderr,"  -r         raw mode.  won't normalize the picture\n");
}

//
// parseargs: parse cli
//
static int parseargs(int argc, char **argv)
{
   char cc;
   extern char *optarg;

   errno=0;

   while ((cc = getopt(argc, argv, "gho:qr")) != EOF)
   {
      switch (cc) {
         case 'g':
            gFlagDrawGrid = 1;
            break;
         case 'h':
            gFlagUserCliValid = 1;
            gFlagUserCliHelp = 1;
            break;
         case 'o':
            if(1 == gFlagUserCliModeSelected)
            {
               fprintf(stderr,"ERROR: can't specify quiet mode and an output path!\n");
               gFlagUserCliValid=0;
               // FIXME: is this the right errno?
               errno=E2BIG;
               break;
            }
            if(OUTPATH_MAX_LEN < strlen(optarg))
            {
               fprintf(stderr,"ERROR: path too long!\n");
               errno=ENAMETOOLONG;
               break;
            }
            gFlagUserCliValid = 1;
            gFlagNoWriteVideo = 0;
            gFlagUserCliModeSelected = 1;
            strncpy(gOutpath, optarg, OUTPATH_MAX_LEN);
            gOutpath[strlen(optarg)] = '\0';
            break;
         case 'q':
            if(1 == gFlagUserCliModeSelected)
            {
               fprintf(stderr,"ERROR: can't specify quiet mode and an output path!\n");
               gFlagUserCliValid=0;
               // FIXME: is this the right errno?
               errno=E2BIG;
               break;
            }
            gFlagUserCliValid = 1;
            gFlagNoWriteVideo = 1;
            gFlagUserCliModeSelected = 1;
            break;
         case 'r':
            gFlagRawMode = 1;
            break;
         default:
            errno=EINVAL;
            break;
      }
   }

   return(errno);
}

//
// terminate: signal handler to cleanup the camera connection and exit
//
static void terminate(int xx)
{
   fclose(gOutfilefps);
   // TODO: perhaps we shouldn't ALWAYS exit with 0
   exit(0);
}

