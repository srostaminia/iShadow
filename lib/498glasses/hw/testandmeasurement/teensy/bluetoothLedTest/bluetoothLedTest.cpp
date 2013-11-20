//**************************************************************************************************
// includes
// teensy and arduino stuff
#include "pins_arduino.h"
#include "core_pins.h"
#include "usb_serial.h"
#include "HardwareSerial.h"


//**************************************************************************************************
// global constants
#define BAUDRATE  (115200)
const unsigned int  PINLED  = 13;


HardwareSerial Uart = HardwareSerial();

char     txch;
unsigned ledst;


//**************************************************************************************************
// function prototypes
void setup();
void loop();


//**************************************************************************************************
// function definitions
// arduino-ish main
extern "C" int main(void)
{
   // Arduino's main() function just calls setup() and loop()....
   setup();
   while (1) {
      loop();
      yield();
   }
}

void setup()
{
  Uart.begin(BAUDRATE);
  pinMode(PINLED,OUTPUT);
  digitalWrite(PINLED,1);

/*  txch='A';
  ledst=0;*/
}

void loop()
{
/*  digitalWrite(PINLED,ledst);
  ledst^=1;

  Serial.write(txch++);
  if(txch>'Z')
  {
    txch='A';
  }

  delay(500);*/

  char cc;
  while(!Uart.available());
  cc=Uart.read();
  if('H' == cc)
  {
    digitalWrite(PINLED,1);
    Uart.println("on");
  }
  else
  {
    digitalWrite(PINLED,0);
    Uart.println("off");
  }
}

