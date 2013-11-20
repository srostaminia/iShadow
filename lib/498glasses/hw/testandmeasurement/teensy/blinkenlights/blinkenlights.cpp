//**************************************************************************************************
// includes
// teensy and arduino stuff
#include "pins_arduino.h"
#include "core_pins.h"
#include "usb_serial.h"
#include "HardwareSerial.h"


//**************************************************************************************************
// global constants
int ledPin = 13;


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
  pinMode(ledPin, OUTPUT);
}

void loop()
{
  // FIXME: I only work when the system is doing nothing else
  digitalWrite(ledPin,HIGH);
  delay(500);
  digitalWrite(ledPin,LOW);
  delay(500);
}

