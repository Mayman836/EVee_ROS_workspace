#include <ros.h>
#include <custom_msgs/Waypoint.h>
#include <BluetoothNode.h>

extern ros::NodeHandle nh;

extern ros::Publisher wp_pub;

extern custom_msgs::Waypoint wp_msg;

WaypointList wp_list[MAX_WAYPOINTS];

int wp_index = 0;
int wp_count = 0;

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
    if(wp_count >= MAX_WAYPOINTS) break;

    int32_t latE7 = data[i] | (data[i+1] << 8) | (data[i+2] << 16) | (data[i+3] << 24);

    int32_t lngE7 = data[i+4] | (data[i+5] << 8) | (data[i+6] << 16) | (data[i+7] << 24);

    float lat = latE7/1e7;

    float lng = lngE7/1e7;

    wp_list[wp_count].lat = lat;

    wp_list[wp_count].lng = lng;

    wp_count++;

    nh.spinOnce();
  }
}