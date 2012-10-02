#include <Arduino.h>
#include <SPI.h>
#include "RF24.h"
#include "Payloads.h"

class RadioControl{
  private: 
    RF24 *radio;
    byte srcId;
    byte dstId;
    uint64_t dstRadioId;
    uint64_t srcRadioId;
    byte packetCount;
  
  public: 
    RadioControl(byte src,byte dst);
    ~RadioControl();
    void initialize();
    static uint64_t calcBadgeRadioId(byte id);
    void setSrc(byte src);
    void setDst(byte dst);
    boolean transmitPayload(BadgePayload_t *payload);
    boolean listenForDuration(unsigned long int timeOutLength,BadgePayload_t *payload);
    boolean calcCheckSum(BadgePayload_t *p);
};

RadioControl::RadioControl(byte src,byte dst){
  radio = new RF24(A0,A1);
  srcId = src;
  dstId = dst;
  packetCount = 0;
}

RadioControl::~RadioControl(){
  delete(radio);
}

void RadioControl::initialize(){
  radio->begin();
  radio->setAutoAck(true);
  radio->setRetries(15,15);
  radio->setPALevel(RF24_PA_HIGH);  //Hope to increase Range
  radio->setDataRate(RF24_250KBPS); //To increase Range
  radio->setPayloadSize(sizeof(BadgePayload_t));
  radio->openWritingPipe(calcBadgeRadioId(dstId));
  radio->openReadingPipe(1,calcBadgeRadioId(srcId));
  radio->startListening();
}

void RadioControl::setSrc(byte src){
   srcId = src;
   srcRadioId = calcBadgeRadioId(srcId);
   radio->openReadingPipe(1,srcRadioId);
}

void RadioControl::setDst(byte dst){
  dstId = dst;
  dstRadioId = calcBadgeRadioId(dstId);
  radio->openWritingPipe(dstRadioId);
}

boolean RadioControl::transmitPayload(BadgePayload_t *payload){
  payload->packetId = packetCount++;
  payload->originId = srcId;
  payload->destId = dstId;
  calcCheckSum(payload);
  radio->stopListening();
  boolean success =  radio->write(payload,sizeof(BadgePayload_t));
  radio->startListening();
  
  Serial.print("TX packet to:");
  Serial.print(payload->destId);
  Serial.print(" p:");
  Serial.print(payload->packetId);
  Serial.print(" ok:");
  Serial.println(success);
  return success;
}

boolean RadioControl::listenForDuration(unsigned long int timeOutLength,BadgePayload_t *payload){
  boolean success = false;
  unsigned long started_waiting_at = millis();
  boolean timeout = false;
  
  while ( ! radio->available() && ! timeout )
    if (millis() - started_waiting_at > timeOutLength )
        timeout = true;
  if (!timeout){
    success = true;
    radio->read(payload, sizeof(BadgePayload_t));     
  }
  return success;
}

boolean RadioControl::calcCheckSum(BadgePayload_t *p){
  int sum = p->originId;
  sum += p->packetId;
  sum += p->commandId;
  for(int i=0;i<maxDifferencesPerPacket;i++){
    sum += p->connections[i].id + p->connections[i].count;
  }
  p->checksum = (byte)(0xFF & sum);
}

uint64_t RadioControl::calcBadgeRadioId(byte id){
  uint64_t num = 0x11223344FFLL;
  if(id == 0){
    //base1
    num = 0x6261736531LL;
  } else {
    num = num && (uint64_t)id;
  }
  return num;
}


