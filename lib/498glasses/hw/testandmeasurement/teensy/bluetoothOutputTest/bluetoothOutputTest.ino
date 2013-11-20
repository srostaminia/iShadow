#define BAUDRATE  (115200)
const unsigned int  PINLED  = 13;


HardwareSerial Uart = HardwareSerial();

char     txch;
unsigned ledst;


void setup()
{
  Uart.begin(BAUDRATE);
  pinMode(PINLED,OUTPUT);
  digitalWrite(PINLED,1);

  txch='A';
  ledst=0;
}

void loop()
{
  digitalWrite(PINLED,ledst);
  ledst^=1;

  Uart.write(txch++);
  if(txch>'Z')
  {
    txch='A';
  }

  delay(500);
}

