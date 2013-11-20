//**************************************************************************************************
// includes
// teensy and arduino stuff
#include "pins_arduino.h"
#include "core_pins.h"
#include "usb_serial.h"
#include "HardwareSerial.h"


//**************************************************************************************************
// global constants
#define ESCAPE     ("^#^$^%")
#define BAUDRATE   (460800)
#define BAUDRATE2  (921600)


const unsigned int  PINLED = 13;
const unsigned int  BT_EN  = 21;


HardwareSerial Uart = HardwareSerial();

unsigned flaginit;
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
  ledst=1;
  pinMode(BT_EN, OUTPUT);
  digitalWrite(BT_EN, 0);

  flaginit=0;
}

void loop()
{
  unsigned ii;
  char cc;


  if(0==flaginit)
  {
    for(ii=0;ii<3;++ii)
    {
      while(Uart.available())
      {
        cc=Uart.read();
        Serial.write(cc);
      }
      delay(1000);
    }

    // enter command mode
    Uart.println(ESCAPE);
    Serial.write("tx: ");
    Serial.println(ESCAPE);

    Serial.write("rx: ");
    while(!Uart.available());
    while(Uart.available())
    {
      cc=Uart.read();
      Serial.write(cc);
      delay(5);
    }

    delay(1000);

    // disable streaming serial (for flow control)
    Uart.println("AT+AB Config StreamingSerial = False");
    Serial.write("tx: ");
    Serial.println("AT+AB Config StreamingSerial = False");

    Serial.write("rx: ");
    while(!Uart.available());
    while(Uart.available())
    {
      cc=Uart.read();
      Serial.write(cc);
      delay(5);
    }

    delay(1000);

    // set CPU MHz to highest (64MHz)
    Uart.println("AT+AB Config CpuMHz = 64");
    Serial.write("tx: ");
    Serial.println("AT+AB Config CpuMHz = 64");

    Serial.write("rx: ");
    while(!Uart.available());
    while(Uart.available())
    {
      cc=Uart.read();
      Serial.write(cc);
      delay(5);
    }

    delay(1000);

    // change device name to InSight
    Uart.println("AT+AB Config DeviceName = InSight");
    Serial.write("tx: ");
    Serial.println("AT+AB Config DeviceName InSight");

    Serial.write("rx: ");
    while(!Uart.available());
    while(Uart.available())
    {
      cc=Uart.read();
      Serial.write(cc);
      delay(5);
    }

    delay(1000);

    // change baud
    Uart.write("AT+AB ChangeDefaultBaud ");
    Uart.println(BAUDRATE2);
    Serial.write("tx: ");
    Serial.write("AT+AB ChangeDefaultBaud ");
    Serial.println(BAUDRATE2);

    Serial.write("rx: ");
    while(!Uart.available());
    while(Uart.available())
    {
      cc=Uart.read();
      Serial.write(cc);
      delay(5);
    }

    delay(1000);
    Uart.begin(BAUDRATE2);

    // exit command mode
    Uart.println("AT+AB Bypass");
    Serial.write("tx: ");
    Serial.println("AT+AB Bypass");

    Serial.write("rx: ");
    while(!Uart.available());
    while(Uart.available())
    {
      cc=Uart.read();
      Serial.write(cc);
      delay(5);
    }


    flaginit=1;
  }
}

