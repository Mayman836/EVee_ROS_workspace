#include <Arduino.h>
#include <ros.h>
#include <custom_msgs/Waypoint.h>

// bluetooth_node (pub)

ros::NodeHandle nh;

custom_msgs::Waypoint wp_msg;

ros::Publisher pub("/navigation/goal", &wp_msg);

void setup() {
  nh.getHardware()->setBaud(115200);
  nh.initNode();

  while (!nh.connected()) {
    nh.spinOnce();
    delay(10);
  }

  nh.advertise(pub);

  delay(100);
}

void loop() {
  if(nh.connected()) {
    wp_msg.lat = 30.0000001;
    wp_msg.lng = 31.0000001;
    pub.publish(&wp_msg);
  }
  nh.spinOnce();
  delay(1000);
}

// #include <Arduino.h>
// #include <NimBLEDevice.h>

// #define DEVICE_NAME "evee_01"
// #define SERVICE_UUID "e4013106-3f4c-4604-ba59-4679b7096414"
// #define WRITE_CHAR_UUID "bb65fda4-7723-497f-a10b-6b0c72896d65"

// NimBLEServer* pServer = nullptr;
// NimBLEService* pService = nullptr;
// NimBLECharacteristic* pWriteChar = nullptr;

// bool deviceConnected = false;
// bool rideInProgress = false;
// bool waypointBufferIsReady = false;

// std::string waypointBuffer = "";

// class ServerCallbacks : public NimBLEServerCallbacks {
//   void onConnect(NimBLEServer* server, NimBLEConnInfo& connInfo) override {
//     deviceConnected = true;
//   }

//   void onDisconnect(NimBLEServer* server, NimBLEConnInfo& connInfo, int reason) override {
//     deviceConnected = false;
//     NimBLEDevice::startAdvertising();
//   }
// };

// class WriteCallbacks : public NimBLECharacteristicCallbacks {
//   void onWrite(NimBLECharacteristic* characteristic, NimBLEConnInfo& connInfo) override {
//     std::string value = characteristic->getValue();

//     if (value == "GO") {
//       rideInProgress = true;
//     } else if (value == "STOP") {
//       rideInProgress = false;
//     } else if (value == "DONE") {
//       waypointBufferIsReady= true;
//     } else if (value == "CLEAR") {
//       waypointBuffer = "";
//     } else {
//       waypointBuffer += value;
//     }
//   }
// };

// void decodeWaypoints() {
//   const uint8_t* data = (const uint8_t*)waypointBuffer.data();
//   int len = waypointBuffer.size();

//   for(int i; i < len; i += 8) {
//     int32_t latE7 = data[i] | (data[i+1] << 8) | (data[i+2] << 16) | (data[i+3] << 24);
//     int32_t lngE7 = data[i+4] | (data[i+5] << 8) | (data[i+6] << 16) | (data[i+7] << 24);
//     float lat = latE7/1e7;
//     float lng = lngE7/1e7;
//   }
// }

// void setup() {
//   Serial.begin(115200);

//   NimBLEDevice::init(DEVICE_NAME);

//   pServer = NimBLEDevice::createServer();
//   pServer->setCallbacks(new ServerCallbacks());

//   pService = pServer->createService(SERVICE_UUID);

//   pWriteChar = pService->createCharacteristic(WRITE_CHAR_UUID, NIMBLE_PROPERTY::WRITE);
//   pWriteChar->setCallbacks(new WriteCallbacks());

//   pService->start();

//   NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();

//   pAdvertising->addServiceUUID(SERVICE_UUID);

//   NimBLEDevice::startAdvertising();
// }

// void loop() {
//   if(!waypointBuffer.empty() && waypointBufferIsReady){
//     decodeWaypoints();
//     waypointBuffer = "";
//     waypointBufferIsReady = false;
//   }
// }