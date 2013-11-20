#include <ArduEye_SMH.h>
#include <CYE_Images_v1.h> 
#include <SPI.h>

// ARDUINO ONBOARD ADC
#define SMH1_ADCTYPE_ONBOARD 0
short chipSelect=0; //which vision chip to read from
unsigned char adcType=SMH1_ADCTYPE_ONBOARD;
int count;
char gain;
char useamp;
short vref;
char adc_type;
unsigned char aref;
int avcPin;
int nr;
int nc;

//=======================================================================
// ARDUINO SETUP AND LOOP FUNCTIONS

void setup()
{
  avcPin=3;
  
  // initialize serial port
  Serial.begin(115200); //GUI defaults to this baud rate
  Serial.print("R\n");  
  
  //analogReference(EXTERNAL);
  pinMode(avcPin,OUTPUT); //use for pwm
  pinMode(A5,INPUT); //use for pwm check
  TCCR2B = TCCR2B & 0b11111000 | 0x01;   // PWM 3 & 11 @ 32 kHz
    
  //analogWrite(avcPin,8*4);
  //delay(200);
  //for(int i; i<10000; i++){   
  //   Serial.println(analogRead(A5),DEC);
  //   delay(100);
  //}    
    
  //initialize SPI (needed for external ADC
  SPI.begin();  
    
  //initialize ArduEye Stonyman
  ArduEyeSMH.begin();
  //pinMode(13, OUTPUT); 

  count=0;
  gain=6;
  useamp = 1;
  vref=28;
  adc_type = 0;
  aref = 8*4;
  nr =20;
  nc=20;
  

  ArduEyeSMH.setAmpGain(gain); 
  ArduEyeSMH.setVREF(vref);    

  //analogReference(EXTERNAL);

 
}

void loop(){

  digitalWrite(avcPin,LOW);  
  
  char command;
    if (Serial.available() > 0) {
        command = Serial.read();
        switch(command){
          case 'n':
            streamToSerial(count,adc_type,0,useamp);
            count=count+1;
            break;
          case 'r':  
            count=0;
            ArduEyeSMH.setPointerValue(SMH_SYS_ROWSEL,count); 
            break; 
          case 'g':
            gain = (char) Serial.parseInt();
            ArduEyeSMH.setAmpGain(gain); 
            //Serial.print('g');
            //Serial.println(gain,DEC);
            break;
          case 'a':
            useamp = (char) Serial.parseInt();
            //Serial.print('a');
            //Serial.println(useamp,DEC);
            break;
          case 'v':
            vref = (char) Serial.parseInt();
            ArduEyeSMH.setVREF(vref);    
            //Serial.print('a');
            //Serial.println(useamp,DEC);
            break;
          case 'd':
            adc_type = (char) Serial.parseInt();
            break;
          case 'f':
            aref = 63 + 27*((char) Serial.parseInt());
            break;

        }
    }  
    delay(10);
}


void streamToSerial(int count, char adc_type, unsigned char anain,char useAmp){ 
  unsigned char val_low[nc];
  unsigned char val_hi[nc];
  int val;

    //For external ADC
    if(adc_type==0){
      ArduEyeSMH.setAnalogInput(anain);
      digitalWrite(avcPin,LOW);
      analogWrite(avcPin,aref);
      delay(200);
    }  
    else if(adc_type==1){
      ArduEyeSMH.setADCInput(anain,1); // enable chip
      ADC_SS_PORT |= ADC_SS; // make sure SS is high
    }

    ArduEyeSMH.setPointerValue(SMH_SYS_ROWSEL,0); 
    for (int row=0;row<nr;row++){
    ArduEyeSMH.setPointerValue(SMH_SYS_COLSEL,0);
    for (int col=0; col<nc; col++) {
      // settling delay
      delayMicroseconds(1);
      
      // pulse amplifier if needed
      if (useAmp && gain >0) {
        ArduEyeSMH.pulseInphi(2);
        delayMicroseconds(1);
      }
      
      if(adc_type==0){
        //onboard 10 bit ADC
        val = analogRead(anain);
        val_hi[col] = (val/256)&3;  //split 10 bit adc output into two values
        val_low[col] = val%256; 
      }    
      else if(adc_type==1){
        // Microchip 12 bit ADC on rocketshield
        ADC_SS_PORT &= ~ADC_SS; // turn SS low to start conversion
        val_hi[col] =((unsigned char)SPI.transfer(0)); // get high byte and mask upper bits
        val_low[col]=((unsigned char)SPI.transfer(0));//>>1; // get low byte and shift out last bit
        ADC_SS_PORT |= ADC_SS; // SS high to stop
        
        
        //Serial.print(val_hi[col],BIN);
        //Serial.print(",");

        //Serial.print(val_low[col],BIN);
        //Serial.print(",");

        val_hi[col] =val_hi[col] & 0x1F; // get high byte and mask upper bits
        val_low[col]=val_low[col]>>1; // get low byte and shift out last bit

        val_low[col]= val_low[col] | ((val_hi[col]%1)*128) ;//set top bit of low to bottom bit of hi
        val_hi[col] = val_hi[col]>>1;//shift high byte by leave 4 valif bits 

        val = (int)val_hi[col];
        val = val*256 + (int)val_low[col];

        //Serial.print(val,BIN);
        //Serial.print(" ");

      }      
      ArduEyeSMH.incValue(1);
    }
    Serial.write(val_low,nc);
    Serial.write(val_hi,nc);
    //Serial.println(" ");
    
    ArduEyeSMH.setPointer(SMH_SYS_ROWSEL);
    ArduEyeSMH.incValue(1);  
    delayMicroseconds(1); 
    }
    
    digitalWrite(avcPin,LOW);

    //For external ADC
    if(adc_type==1){
      ArduEyeSMH.setADCInput(anain,0); // disable chip
    }
}

