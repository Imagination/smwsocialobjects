#include <Arduino.h>
#define maxSerialBufferSize 40

class SerialControl {
  private:
    char serialBuffer[maxSerialBufferSize+1];
    unsigned int bufferIndex;
    char shiftSerialBufferDown(int amount);
    RadioControl *rf;
  
  public:
    SerialControl(RadioControl *aRadioControl);
    void readSerialPort();
    void clearBuffer();
    void parseSerialBuffer();
    void transmitCommand(byte *values);
};

SerialControl::SerialControl(RadioControl *radioControl){
  rf = radioControl;
  bufferIndex = 0;
}

void SerialControl::readSerialPort(){  
  //Get serial in for command from serial port
  boolean serialIncoming = false;
  while(Serial.available()){
    serialIncoming = true;
    serialBuffer[bufferIndex++] = Serial.read();
    while(bufferIndex>=maxSerialBufferSize){
      shiftSerialBufferDown(1);
    }
  }
  if(serialIncoming){
    parseSerialBuffer();
  }
}
 
char SerialControl::shiftSerialBufferDown(int amount){
  int i;
  for(i=0;i<maxSerialBufferSize-amount;i++){
    serialBuffer[i] = serialBuffer[i+amount];
  }
  for(i=maxSerialBufferSize-amount;i<maxSerialBufferSize;i++){
    serialBuffer[i] = 0;
  }
  bufferIndex = bufferIndex-amount;
}
 
void SerialControl::clearBuffer(){
  bufferIndex = 0;
  for(int i=0;i<maxSerialBufferSize;i++){
    serialBuffer[i] = 0;
  }
}
 
 
// Parse commands from the serial buffer
// M - meet
// U - update old connections
void SerialControl::parseSerialBuffer(){
  String str1(serialBuffer);
  int cIndex = str1.indexOf("c");
  if(cIndex>-1){
    shiftSerialBufferDown(cIndex);
    String str2(serialBuffer);
    int eolIndex = str2.indexOf("\n");
    if(eolIndex>-1){
      shiftSerialBufferDown(eolIndex+1);
      str2 = str2.substring(1,eolIndex);
      Serial.print("Command String:");
      Serial.print(str2);
      Serial.println("");
      if(str2.length() >= 3){
        byte values[7] = {0,0,0,0,0,0,0};
        boolean endReached = false;
        boolean invalid = false;
        int count = 0;
        int valueCount = 0;
        int num = 0;
        byte ch;
        for(int count=0; (count<str2.length())  && (!invalid) && (valueCount < 7) ; count++){
          ch = str2.charAt(count);
          if(ch>='0' && ch <='9'){
            num *= 10;
            num += ch - '0';
            values[valueCount] = (byte)(0xFF & num);
          } else if(ch == ',') {
            valueCount += 1;
            num = 0;
          } else {
            Serial.println("Invalid Command");
            invalid = true;
          }
          
        } //End of While
        if(!invalid){
          
          //DEBUG
          Serial.print("Values:");
          for(int i=0;i<7;i++){
            Serial.print(values[i]);
            if(i!=6){
              Serial.print(",");
            }
          }
          Serial.println("");
          //END DEBUG
          
          transmitCommand(values);
        }
      }
    }  
  }
}

void SerialControl::transmitCommand(byte *values){
  CommandPayload_t payload;
  payload.originId = 0x00;
  payload.packetId = 0x00;
  payload.checksum = 0x00;
  payload.commandId = values[1];
  payload.redValue = values[2];
  payload.greenValue = values[3];
  payload.blueValue = values[4];
  payload.info[0] = values[5];
  payload.info[1] = values[6];
  payload.info[2] = values[7];
  payload.info[3] = values[8];
  payload.info[4] = values[9];
  
  rf->setDst(values[0]);
  rf->transmitPayload((BadgePayload_t *)&payload);
  
  Serial.print("Sent Command to:");
  Serial.print(values[0],HEX);
  Serial.print(" cmd:");
  Serial.println(payload.commandId,HEX);
}
