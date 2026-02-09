#ifndef BLUETOOTH_NODE_H
#define BLUETOOTH_NODE_H

#include <NimBLEDevice.h>
#include <string>

#define DEVICE_NAME "evee_01"
#define SERVICE_UUID "e4013106-3f4c-4604-ba59-4679b7096414"
#define WRITE_CHAR_UUID "bb65fda4-7723-497f-a10b-6b0c72896d65"

extern bool drive;
extern bool waypointBufferIsReady;

extern uint16_t bleConnHandle;

extern NimBLEServer* pServer;
extern NimBLEService* pService;
extern NimBLECharacteristic* pWriteChar;

extern std::string waypointBuffer;

class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* server, NimBLEConnInfo& connInfo) override;
  void onDisconnect(NimBLEServer* server, NimBLEConnInfo& connInfo, int reason) override;
};

class WriteCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* characteristic, NimBLEConnInfo& connInfo) override;
};

void setupBLE();

void disconnectBLE();

void decodeWaypoints();

#endif