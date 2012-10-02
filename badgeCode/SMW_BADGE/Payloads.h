#ifndef PAYLOADS_HEADERS 
#define PAYLOADS_HEADERS

#define maxConnections 150
#define maxCount 250
#define maxSerialBufferSize 40
#define maxDifferencesPerPacket 5
#define irHitToLogRatio 4

//Data Structures for rf packets
struct connectData_t {
  byte id;
  byte count;
}; 
 
struct BadgePayload_t
{
  byte originId;
  byte packetId;
  byte commandId;
  byte destId;
  byte routeId;
  
  connectData_t connections[maxDifferencesPerPacket]; //2 bytes per connection
  byte checksum;
};
 
//Base acknowledgement packet
struct BaseAckPayload_t
{
  byte originId;
  byte packetId;
  byte commandId;
  byte destId;
  byte routeId;
  
  byte ackPacketId;
  
  byte padding[maxDifferencesPerPacket*2-1];
  byte checksum;
};


struct CommandPayload_t 
{
  byte originId;
  byte packetId;
  byte commandId;
  byte destId;
  byte routeId;
  
  byte redValue;
  byte blueValue;
  byte greenValue;
  
  byte info[maxDifferencesPerPacket*2 - 3];
  byte checksum;
};

#endif
