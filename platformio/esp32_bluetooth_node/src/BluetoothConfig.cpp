#include <BluetoothConfig.h>
#include <BluetoothFunc.h>

bool deviceConnected = false;
bool rideInProgress = false;
bool waypointBufferIsReady = false;

NimBLEServer* pServer = nullptr;
NimBLEService* pService = nullptr;
NimBLECharacteristic* pWriteChar = nullptr;

std::string waypointBuffer = "";

void setupBLE() {
  NimBLEDevice::init(DEVICE_NAME);

  pServer = NimBLEDevice::createServer();

  pServer->setCallbacks(new ServerCallbacks());

  pService = pServer->createService(SERVICE_UUID);

  pWriteChar = pService->createCharacteristic(WRITE_CHAR_UUID, NIMBLE_PROPERTY::WRITE);

  pWriteChar->setCallbacks(new WriteCallbacks());

  pService->start();

  NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();

  pAdvertising->addServiceUUID(SERVICE_UUID);

  NimBLEDevice::startAdvertising();
}