#define ESCAPE     ("^#^$^%")
#define BAUDRATE   (115200)
#define BAUDRATE2  (115200)


const unsigned int  PINLED = 13;
const unsigned int  BT_EN  = 21;


HardwareSerial Uart = HardwareSerial();

unsigned flaginit;
unsigned ledst;


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

  // enter command mode
  Uart.println(ESCAPE);
  Serial.write("tx: ");
  Serial.println(ESCAPE);

  Serial.write("rx: ");
  for(ii=0; (ii<5)&&(!Uart.available()); ++ii)
  {
    delay(100);
  }
  while(Uart.available())
  {
    cc=Uart.read();
    Serial.write(cc);
    delay(5);
  }

    delay(1000);
/*
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

    // change baud
    Uart.write("AT+AB ChangeBaud ");
    Uart.println(BAUDRATE2);
    Serial.write("tx: ");
    Serial.write("AT+AB ChangeBaud ");
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
*/
/*
  while(!Uart.available());
  cc=Uart.read();
  if('p' == cc)
  {
    delay(1000);
    Uart.write("tx: ");
    Uart.println(ESCAPE);
    Uart.write("rx: ");
    Uart.println(resp1);
    Uart.write("tx: ");
    Uart.println("AT+AB Bypass");
    Uart.write("rx: ");
    Uart.println(resp2);

    ledst^=1;
    digitalWrite(PINLED,ledst);
  }*/
}

