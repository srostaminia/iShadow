#include "stony.h"

// teensy and arduino stuff
#include "pins_arduino.h"
#include "core_pins.h"
#include "usb_serial.h"
//#include "HardwareSerial.h"


// thanks to Boyan
//#define SMH_VREF_3V3    41     //vref for 3.3 volt
//#define SMH_NBIAS_3V3   50     //nbias for 3.3 volts
//#define SMH_AOBIAS_3V3  37     //aobias for 3.3 volts
//#define SMH_GAIN_3V3    2//3   //amp gain for 3.3 volts 
//#define SMH_SELAMP_3V3  1


//#define SMH_VREF_3V3    30     //vref for 3.3 volt
//#define SMH_NBIAS_3V3   55     //nbias for 3.3 volts
//#define SMH_AOBIAS_3V3  55     //aobias for 3.3 volts
//#define SMH_GAIN_3V3    0
//#define SMH_SELAMP_3V3  0

#define SMH_VREF_3V3    47     //vref for 3.3 volt
#define SMH_NBIAS_3V3   50     //nbias for 3.3 volts
#define SMH_AOBIAS_3V3  42     //aobias for 3.3 volts
#define SMH_GAIN_3V3    5
#define SMH_SELAMP_3V3  1

const char PIN_LED = 13;

const char PIN_RESP   = 2;
const char PIN_INCP   = 3;
const char PIN_RESV   = 4;
const char PIN_INCV   = 5;
const char PIN_INPHI  = 6;
//const char PIN_ANALOG = A8;	// inward-facing
const char PIN_ANALOG = A9;	// outward-facing
const char PIN_FLAG = 1;

const unsigned long BAUDRATE = 57600;


Stonyman stonycam;
short config_state;

// for bluetooth over UART
//HardwareSerial Uart = HardwareSerial();


//**************************************************************************************************
// function prototypes
void setup();
void loop();
void getCmd(char*);
void frameCaptureAndTx();
void subsampleCaptureAndTx();
short invert_state();


//**************************************************************************************************
// function definitions
// arduino-ish main
extern "C" int main(void)
{
   config_state = 1;

   // Arduino's main() function just calls setup() and loop()....
   setup();
   while (1) {
      loop();
      yield();
   }
}

void setup()
{
   Serial.begin(BAUDRATE);
   pinMode(13,OUTPUT);
   pinMode(1,OUTPUT);
   digitalWrite(1,0);
   digitalWrite(PIN_LED,1);

   // raw: for 5v0
   //(void)stonycam.init( PIN_RESP, PIN_INCP, PIN_RESV, PIN_INCV, PIN_INPHI, PIN_ANALOG,
   //                     Stonyman::DEFAULT_VREF_5V0, Stonyman::DEFAULT_NBIAS_5V0,
   //                     Stonyman::DEFAULT_AOBIAS_5V0, Stonyman::DEFAULT_GAIN_5V0,
   //                     Stonyman::DEFAULT_SELAMP_5V0 );
   // amp: for 3v3
   //(void)stonycam.init( PIN_RESP, PIN_INCP, PIN_RESV, PIN_INCV, PIN_INPHI, PIN_ANALOG,
   //                     35,50,50,1,1); // amp: for 3v3
   // russ: Boyan's values
   (void)stonycam.init( PIN_RESP, PIN_INCP, PIN_RESV, PIN_INCV, PIN_INPHI, PIN_ANALOG,
                        SMH_VREF_3V3,SMH_NBIAS_3V3,SMH_AOBIAS_3V3,SMH_GAIN_3V3,
                        SMH_SELAMP_3V3);
}

void loop()
{
   char cmd;

   while(!Serial.available());

   digitalWrite(PIN_LED,0);

   getCmd(&cmd);

   switch(cmd)
   {
      case 'Z':
         //stonycam.chipToMatlab();
	 frameCaptureAndTx();
	 //subsampleCaptureAndTx();
	 //config_state = invert_state();
         break;
      case '?':
      default:
         Serial.println("Z: MATLAB chip"); 
         delay(50);
         break;
   }

   digitalWrite(PIN_LED,1);
}

short invert_state()
{
    if (config_state == 1) {
	stonycam.clearTestConfig();
	return 0;
    } else {
	stonycam.setTestConfig();
	return 1;
    }
}

// russ: this is a (simplified) clone of ArduEyeGUIClass::getCommand from the ArduEye_GUI library
// for some reason, sscanf is blowing up my linker (probably a bug in teensyduino for teensy3),
// so I just removed the argument processing (which I didn't need anyway)
void getCmd(char *command)
{
   char cmdbuf[11];
   unsigned char i;

   // initialize
   for (i=0; i<11; ++i)
      cmdbuf[i] = 0;
   i = 0;
   // delay to ensure that all stuff is sent through serial port
   delay(100);
   // load cmdbuf
   while (Serial.available() && i<10) {
      cmdbuf[i] = Serial.read();
      i++;
   }
   // clear end of array
   cmdbuf[10]=0;
   // clear rest of buffer
   while (Serial.available())
      ;
   // get command
   *command = cmdbuf[0];
}

void frameCaptureAndTx()
{
   short ii,jj,mm;
   short imrow[112];
   char  imrowsc[112];

   digitalWriteFast(PIN_FLAG,1);
   // russ: for debugging
   //for(ii=0; ii<112*112; ++ii) Serial.print((char)0);
   for(ii = 0; ii < 56; ++ii)
   {
      for(mm = 0; mm < 112; ++mm)
      {
      	imrow[mm] = 0;
      }
      stonycam.getImage(imrow,ii,1,1,0,112,1);
      // FIXME russ: not efficient
      for(jj = 0; jj < 56; ++jj)
      {
         //imrowsc[jj]=255-imrow[jj]+stonymask[(112*ii)+jj];
         //imrowsc[jj]=imrow[jj]-stonymask[(112*ii)+jj];
         // russ: try this if the masked version is not working
         //imrowsc[jj]=128-imrow[jj];
         
	// addison: use this one!
	//if ((char)(imrow[jj]) == '\n')
	//	Serial.print((char)1);
	//else
		//if(imrow[jj]>255){
		//	delay(500);
		//	digitalWrite(PIN_LED,1);
		//	delay(500);
		//	digitalWrite(PIN_LED,0);
		//}
//		Serial.print((char)(imrow[jj]));
//		Serial.print((char)(imrow[jj]>>8));
//		imrow[jj] = 0;
        Serial.print((char)1);
        Serial.print((char)1);
      }
      //Serial.print((char)ii);
   }
   digitalWriteFast(PIN_FLAG,0);
}

/*void frameCaptureAndTx()
{
   short ii,jj;
   short imrow[112];
   char  imrowsc[112];

   DataConn.print(SYMBOL_SOF);
   DataConn.print(OPCODE_FRAME);

   // russ: for debugging
   //for(ii=0; ii<112*112; ++ii) Serial.print(0);
   for(ii = 0; ii < 112; ++ii)
   {
      stonycam.getImage(imrow,ii,1,1,0,112,1);
      // FIXME russ: not efficient
      for(jj = 0; jj < 112; ++jj)
      {
         //imrowsc[jj]=255-imrow[jj]+stonymask[(112*ii)+jj];
         imrowsc[jj]=imrow[jj]-stonymask[(112*ii)+jj];
         // russ: try this if the masked version is not working
         //imrowsc[jj]=128-imrow[jj];
         DataConn.print((char)(imrowsc[jj]));
      }
   }
}*/

void subsampleCaptureAndTx()
{
   const int NUM_SUBSAMPLE = 69;
   const int PIN_FLAG = 1;
   short i;
   short pixels[NUM_SUBSAMPLE];

   unsigned char rowlocs[] = { 1 , 2 , 2 , 3 , 4 , 4 , 7 , 9 , 14 , 16 , 16 , 19 , 21 , 23 , 30 , 31 , 32 , 32 , 33 , 34 , 34 , 37 , 38 , 41 , 43 , 44 , 49 , 49 , 49 , 51 , 52 , 56 , 56 , 58 , 65 , 68 , 69 , 69 , 70 , 71 , 74 , 75 , 77 , 77 , 78 , 81 , 83 , 83 , 83 , 85 , 87 , 87 , 88 , 88 , 91 , 92 , 93 , 95 , 97 , 101 , 101 , 102 , 102 , 104 , 104 , 108 , 108 , 109 , 111 };
   unsigned char collocs[] = { 54 , 36 , 68 , 54 , 24 , 74 , 54 , 81 , 101 , 3 , 67 , 54 , 93 , 47 , 1 , 101 , 55 , 69 , 54 , 48 , 68 , 54 , 34 , 15 , 22 , 50 , 39 , 67 , 90 , 68 , 22 , 27 , 42 , 54 , 88 , 90 , 7 , 32 , 54 , 60 , 75 , 57 , 12 , 55 , 54 , 85 , 1 , 28 , 64 , 68 , 7 , 69 , 54 , 73 , 54 , 7 , 94 , 50 , 69 , 50 , 84 , 54 , 90 , 52 , 54 , 54 , 72 , 54 , 56 };
   
   digitalWriteFast(PIN_FLAG,1);
   stonycam.getPixelList( pixels, rowlocs, collocs, NUM_SUBSAMPLE );
   digitalWriteFast(PIN_FLAG,0);

   for (i = 0; i < NUM_SUBSAMPLE; i++)
	Serial.print((char)(pixels[i]));
}


