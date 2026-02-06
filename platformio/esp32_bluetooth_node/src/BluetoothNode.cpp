#include <ros.h>
#include <custom_msgs/Waypoint.h>
#include <std_msgs/Bool.h>
#include <BluetoothNode.h>

extern ros::NodeHandle nh;

extern std_msgs::Bool drive_msg;

extern custom_msgs::Waypoint wp_msg;

extern ros::Publisher wp_pub;

bool drive = false;
bool waypointBufferIsReady = false;

std::string waypointBuffer = "";

NimBLEServer* pServer = nullptr;
NimBLEService* pService = nullptr;
NimBLECharacteristic* pWriteChar = nullptr;

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

void ServerCallbacks::onConnect(NimBLEServer* server, NimBLEConnInfo& connInfo) {}

void ServerCallbacks::onDisconnect(NimBLEServer* server, NimBLEConnInfo&, int reason) {
    NimBLEDevice::startAdvertising();
}

void WriteCallbacks::onWrite(NimBLECharacteristic* characteristic, NimBLEConnInfo& connInfo) {
    std::string value = characteristic->getValue();

    if (value == "GO") drive = true;
    else if (value == "STOP") drive = false;
    else if (value == "DONE") waypointBufferIsReady = true;
    else if (value == "CLEAR") waypointBuffer = "";
    else waypointBuffer += value;
}

void decodeWaypoints() {
  const uint8_t* data = (const uint8_t*)waypointBuffer.data();

  int len = waypointBuffer.size();

  for(int i = 0; i < len; i += 8) {
    int32_t latE7 = data[i] | (data[i+1] << 8) | (data[i+2] << 16) | (data[i+3] << 24);

    int32_t lngE7 = data[i+4] | (data[i+5] << 8) | (data[i+6] << 16) | (data[i+7] << 24);

    float lat = latE7/1e7;

    float lng = lngE7/1e7;

    wp_msg.lat = lat;

    wp_msg.lng = lng;

    wp_pub.publish(&wp_msg);

    delay(5);

    nh.spinOnce();
  }
}