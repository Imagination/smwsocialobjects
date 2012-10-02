#include <SPI.h>
#include <IRremote.h>
#include "RF24.h"
#include "RadioControl.h"
#include "Scheduler.h"
#include "SerialControl.h"

byte badgeId = 0x0;
RadioControl rf(badgeId,0x30);
Scheduler scheduler;
SerialControl serialControl(&rf);

void setup(void)
{
  Serial.begin(57600);
  rf.initialize();
  scheduler.addItem(printWaiting,2000);
  scheduler.addItem(rfListen,0);
  scheduler.addItem(readSerial,0);
}

void loop(void){
  scheduler.update();
}

void printWaiting(){
  Serial.println("Waiting");
}

void readSerial(){
  serialControl.readSerialPort();
}

void rfTransmitTest(){
  Serial.println("About to transmit");
  static int counter = 0;
  BadgePayload_t p;
  p.originId = badgeId;
  p.packetId = counter++;
  p.commandId = 0xAA;
  rf.setDst(0x00);
  rf.transmitPayload(&p);
}

void rfListen(){
  // Listen for incoming
  BadgePayload_t p;
  bool incoming = rf.listenForDuration(400,&p);
  if(incoming){
    //0xBB the code for badge data
    if(p.commandId == 0xBB){
      transmitAcknowledgement(p.originId,p.packetId);
      printPayload(&p);
    }
  }
}

void printPayload(BadgePayload_t *payload ){
  Serial.print("%");
  Serial.print(payload->originId);
  Serial.print(":");
  for(int i=0;i<5;i++){
    Serial.print(payload->connections[i].id);
    Serial.print(",");
    Serial.print(payload->connections[i].count);
    if(i!=4){
      Serial.print(";");
    }
  }
  Serial.println("");
}

void transmitAcknowledgement(byte sourceId,byte packetId){
  //Transmit to badge
  BaseAckPayload_t payload;
  payload.originId = 0x00;
  payload.ackPacketId = packetId;
  payload.checksum = 0x00; //TODO
  payload.commandId = 0xAA;
  // Bit Hacky conversion but all packages same length
  rf.setDst(sourceId);
  rf.transmitPayload((BadgePayload_t *)&payload);
  Serial.print("Sent Acknowlegdement to:");
  Serial.print(sourceId); //,HEX);
  Serial.print(" pkt:");
  Serial.println(payload.ackPacketId); //,HEX);
}
