#include <SPI.h>
const int  cs=10; //chip select 

void setup() {
  // start the SPI library:
  SPI.begin();
  SPI.setBitOrder(MSBFIRST); 
  SPI.setDataMode(SPI_MODE1); // both mode 1 & 3 should work 
  SPI.setClockDivider(SPI_CLOCK_DIV16);

}

void loop() {
  digitalWrite(cs, LOW);
  SPI.transfer(0x34);
  digitalWrite(cs, HIGH);
  delay(100);
  digitalWrite(cs, LOW);
  SPI.transfer(0x52);
  digitalWrite(cs, HIGH);
  delay(100);
}

