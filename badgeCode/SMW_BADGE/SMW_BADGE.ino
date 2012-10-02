
#include <SPI.h>
#include <IRremote.h>
#include "RF24.h"
#include "RadioControl.h"
#include "IR.h"
#include "Scheduler.h"
#include "MeetCounter.h"
#include "LedControl.h"
#include "lights.h"

byte badgeId = 0x83;
int  teamId;

//Alternative bases
#define numBaseAlternatives 5;
byte baseAlternatives[] = {0x00,0xB0,0xB1,0xB2,0xB3};
int baseAlternateIndex = 0;


RadioControl rf(badgeId,0);
IRControl ir(badgeId);
Scheduler scheduler;
MeetCounter meetCounter;
LedControl ledControl;

void setup(void)
{
  Serial.begin(9600);
  rf.initialize();
  ir.initialize();
  teamId = IRControl::getTeamIdFromBadgeId(badgeId);
  scheduler.addItem(irListen,0); // Do it every loop
  scheduler.addItem(rfListen,500);
  scheduler.addItem(transmitIR,1000); //Do it once a second
  scheduler.addItem(updateBase,10000);
}

void loop(void)
{
  ledControl.update();
  scheduler.update(); 
}


void irListen(){
  if(ir.receiveIR()){
    unsigned long int irInCode = ir.getRXIRCode();
    //Serial.print("IR:");
    //Serial.println(irInCode, HEX);
    if((irInCode >= 0xA01)&&(irInCode<=0xA9F)){
      
      Serial.print("IR:");
      Serial.println(irInCode, HEX);

      byte metBadge = 0xFF & irInCode;
      meetCounter.incMeetCount(metBadge);
      int metTeam = IRControl::getTeamIdFromBadgeId(metBadge);
      if(teamId == metTeam){
        ledControl.displayTeamId(teamId);
      }
    }
  }
}

void transmitIR(){
  ir.transmit();
}

void updateBase(){
  BadgePayload_t p;
  int diffs = meetCounter.findDiffsInArrays(&p);
  if(diffs>0){
    transmitDataToBase(&p);
  }
}

void incAlternateIndex(){
  baseAlternateIndex = (baseAlternateIndex +1) % numBaseAlternatives;
}

void transmitDataToBase(BadgePayload_t *badgePayload){
  //BaseAckPayload_t ackPayload;
  bool success = false;
  int attempts = 0;
  int maxAttempts = 6;
  while(!success && attempts<maxAttempts){
    Serial.print("Attempts:");
    Serial.println(attempts);
    attempts++;
    if(attempts == 4){
      incAlternateIndex();
    }
    success = transmitAndWait(badgePayload);
  }
  if(!success){
    Serial.println("Failed Acknowledgement");
    meetCounter.rollback(badgePayload);
    incAlternateIndex();
  } else {
    Serial.println("Successful Acknowledgement");
  }
}

boolean transmitAndWait(BadgePayload_t *badgePayload){
  BaseAckPayload_t ackPayload;
  bool success = false;
  byte dest = baseAlternatives[baseAlternateIndex];
  rf.setDst(dest);
  rf.transmitPayload(badgePayload);
  //Wait random time for response to minimise collisions
  randomSeed(millis());
  long unsigned int wait = random(150,180);
  bool incoming = rf.listenForDuration(wait,(BadgePayload_t *)&ackPayload);
  if(incoming){
    if((ackPayload.commandId == 0xAA) && (ackPayload.ackPacketId == badgePayload->packetId)){
      success = true;
    }
  }
  return success;
}

void rfListen(){
  // Listen for incoming
  BadgePayload_t p;
  CommandPayload_t *cPtr = (CommandPayload_t*)(&p);
  bool incoming = rf.listenForDuration(200,&p);
  if(incoming){
    
 
    //Check command for this badge
    if(p.destId == badgeId){
      
      //DEBUG
    /*
    Serial.println("********************************");
    Serial.print("RxPacket");
    Serial.print(" origin:");
    Serial.print(p.originId);
    Serial.print(" dest:");
    Serial.print(p.destId);
    Serial.print(" packet:");
    Serial.print(p.packetId);
    Serial.print(" cmd:");
    Serial.print(p.commandId);
    Serial.println("");
    Serial.println("********************************");
    */ 
      switch(p.commandId){
        // Set RGB
        case 1:
          Serial.println("RGB Remote Control");
          ledControl.setRGBFromRemote(cPtr->redValue,cPtr->greenValue,cPtr->blueValue);
          break;
        case 2:
          Serial.println("RGB Release Control");
          ledControl.releaseFromRemote();
          break;
        case 3:
          //meetCounter.markAllForChange();
          break;
      }
    }
  }
}
