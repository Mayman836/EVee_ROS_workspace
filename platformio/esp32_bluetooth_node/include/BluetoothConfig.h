#ifndef BLUETOOTH_CONFIG_H
#define BLUETOOTH_CONFIG_H

#include <string>
#include <NimBLEDevice.h>

#define DEVICE_NAME "evee_01"
#define SERVICE_UUID "e4013106-3f4c-4604-ba59-4679b7096414"
#define WRITE_CHAR_UUID "bb65fda4-7723-497f-a10b-6b0c72896d65"

extern NimBLEServer* pServer;
extern NimBLEService* pService;
extern NimBLECharacteristic* pWriteChar;

extern bool deviceConnected;
extern bool rideInProgress;
extern bool waypointBufferIsReady;

extern std::string waypointBuffer;

void setupBLE();

#endif