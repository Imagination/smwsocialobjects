#include "lights.h"

class LedControl {
 private:
   Lights lights;
   unsigned char red,green,blue;
   unsigned long switchOffTime;
   boolean timingActive;
   boolean ledRemoteControlFlag;
   void setLEDs(unsigned char aRed, unsigned char aGreen, unsigned char aBlue);
   void setLEDsForDuration(unsigned long t,unsigned char aRed, unsigned char aGreen, unsigned char aBlue);
 
 public:
   LedControl();
   void update();
   void displayTeamId(int anId);
   void setRGBFromRemote(unsigned char aRed, unsigned char aGreen, unsigned char aBlue);
   void releaseFromRemote();
};

LedControl::LedControl(){
  timingActive = false;
  ledRemoteControlFlag = false;
  red = 0;
  green = 0;
  blue = 0;
  switchOffTime = 0;
}
 
void LedControl::setRGBFromRemote(unsigned char aRed, unsigned char aGreen, unsigned char aBlue){
  ledRemoteControlFlag = true;
  timingActive = false;
  setLEDs(aRed,aGreen,aBlue);
} 

void LedControl::releaseFromRemote(){
  ledRemoteControlFlag = false;
  setLEDs(0,0,0);
}
  
 
void LedControl::update(){
  unsigned long theNow = millis();
  if(timingActive){
    if(theNow > switchOffTime){
      red = 0;
      green = 0;
      blue = 0;
      timingActive = false;
    }
  }
  lights.set(PIN_LED_BOTH, red, green, blue);
}

void LedControl::setLEDs(unsigned char aRed, unsigned char aGreen, unsigned char aBlue){
  red = aRed;
  blue = aBlue;
  green = aGreen;
  
  Serial.print("setLEDs r:");
  Serial.print(red);
  Serial.print(" g:");
  Serial.print(green);
  Serial.print(" b:");
  Serial.println(blue);
  
  for(int i=0;i<6;i++){
    lights.set(PIN_LED_BOTH, red, green, blue);
  }
}
 
void LedControl::setLEDsForDuration(unsigned long t,unsigned char aRed, unsigned char aGreen, unsigned char aBlue){
   unsigned long theNow = millis();
   switchOffTime = theNow + t;
   timingActive = true;
   setLEDs(aRed,aGreen,aBlue);
}
 
void LedControl::displayTeamId(int anId){
   if(!ledRemoteControlFlag){
     bool found = false;
     unsigned char red,blue,green;
   
     switch(anId){
         case 1:
           red = 255;
           green = 60;
           blue = 60;
           found = true;
           break;
         case 2:
           red = 0;
           green = 255;
           blue = 100;
           found = true;
           break;
         case 3:
           red = 0;
           green = 200;
           blue = 255;
           found = true;
           break;
         case 4:
           red = 255;
           green = 255;
           blue = 0;
           found = true;
           break;
     }
     if(found){
        setLEDsForDuration(1000,red, green, blue);
     }
   }
 }
