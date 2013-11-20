#define ESCAPE     ("^#^$^%")
#define BAUDRATE   (115200)
#define BAUDRATE2  (230400)


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

    // setting SPBT interface to SPI
    Uart.println("AT+AB config SPIEnable = True");   
    Serial.write("tx: ");
    Serial.println("AT+AB config SPIEnable = True");
    
    Serial.write("rx: ");
    while(!Uart.available());
    while(Uart.available())
    {
      cc=Uart.read();
      Serial.write(cc);
      delay(5);
    }

    delay(1000);

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

