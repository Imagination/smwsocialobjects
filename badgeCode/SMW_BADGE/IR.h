#include <Arduino.h>
#include <IRremote.h>
#include "pins.h"

IRsend irsend;
IRrecv irrecv(PIN_IR_IN);

class IRControl {
  
  private:
    decode_results results;
    long unsigned int irInCode;
    byte badgeId;
  
  public:
    IRControl(byte aBadgeId){
      badgeId = aBadgeId;
    };
  
    void initialize(){
      irrecv.enableIRIn(); // Start the receiver
      pinMode(PIN_IR_OUT, OUTPUT);
    }; 
  
    boolean receiveIR(){
      boolean success = irrecv.decode(&results);
      irInCode = results.value;
      if(success){
        // Receive the next value
        irrecv.resume();
        //Serial.println("ir in");
        //Serial.println(results.value, HEX);
      } else {
        irInCode = 0;
      }
      return success;  
    }
    
    long unsigned int getRXIRCode(){
      return irInCode;
    }
    
    static int getTeamIdFromBadgeId(byte anId){
      int team = 0;
      if((anId >= 0x00) && (anId <= 0x2f)){
        // ### Team 1 red
        team = 1;
      } else if((anId >= 0x30) && (anId <= 0x5f)){
        // ### Team 2 green
        team = 2;
      } else if((anId >= 0x60) && (anId <= 0x8f)){
        // ### Team 3 blue
        team = 3;
      } else if((anId >= 0x90) && (anId <= 0x9f)){
        // ### Team spare --- spare
        team = 4;
      }
      return team;  
    }
  
    void transmit(){
      for(int i = 0; i < 3; i++) {
        long int code = 0xa00|badgeId;
        irsend.sendSony(code, 12); // ### custom sony IR out
        delay(40);
      }
      delay(20);  
      irrecv.enableIRIn();
    }
};


