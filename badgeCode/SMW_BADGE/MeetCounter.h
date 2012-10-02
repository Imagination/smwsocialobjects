#include "Arduino.h"
#include "Payloads.h"


class MeetCounter {
  
  private:
    //Store of the connections made by badge IR
    int connectionCount[maxConnections];
    byte oldConnectionCount[maxConnections];
  
    //Where the swipe to check for changes has got to
    int updateIndex;
  
  public:
    MeetCounter(){
      updateIndex = 0;
    }
    
    int findDiffsInArrays(BadgePayload_t *payload){
      int diffCount = 0;
      payload->commandId = 0xBB;
      while((diffCount<maxDifferencesPerPacket) && (updateIndex < maxConnections)){
        int nuCount = connectionCount[updateIndex]/irHitToLogRatio;
        if(nuCount!=oldConnectionCount[updateIndex]){
          payload->connections[diffCount].id = updateIndex+1;
          payload->connections[diffCount].count = nuCount;
          oldConnectionCount[updateIndex] = nuCount;
          diffCount++;
        }
        updateIndex++;
      }
      for(int i=diffCount;i<maxDifferencesPerPacket;i++){
        payload->connections[i].id = 0;
        payload->connections[i].count = 0;
      }
      if(updateIndex >= maxConnections){  
        updateIndex = 0;
      }
      return diffCount;
    }
    
    void rollback(BadgePayload_t *payload){
      for(int i=0;i<maxDifferencesPerPacket;i++){
        int id = payload->connections[i].id;
        if(id!=0){
          oldConnectionCount[id-1]=0;
        }
      }
    }
    
    //This function adds a count to each id
    //num is the ID of the found badge
    void incMeetCount(int num){
      if(num>0 && num<=150){
        int value = connectionCount[num-1];
        value += 1;
        if(value >= maxCount * irHitToLogRatio){
          value = maxCount * irHitToLogRatio;
        }
        connectionCount[num-1] = value;
        Serial.print("Increase badge:");
        Serial.print(num);
        Serial.print(" to:");
        Serial.println(connectionCount[num-1]);
      }
    }
};
