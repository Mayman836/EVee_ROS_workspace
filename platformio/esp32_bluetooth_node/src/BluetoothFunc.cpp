#include <ros.h>
#include <custom_msgs/Waypoint.h>
#include <BluetoothFunc.h>
#include <BluetoothConfig.h>

extern ros::NodeHandle nh;

extern ros::Publisher pub;

extern custom_msgs::Waypoint wp_msg;

void ServerCallbacks::onConnect(NimBLEServer* server, NimBLEConnInfo& connInfo) {
    deviceConnected = true;
}

void ServerCallbacks::onDisconnect(NimBLEServer* server, NimBLEConnInfo&, int reason) {
    deviceConnected = false;
    NimBLEDevice::startAdvertising();
}

void WriteCallbacks::onWrite(NimBLECharacteristic* characteristic, NimBLEConnInfo& connInfo) {
    std::string value = characteristic->getValue();

    if (value == "GO") rideInProgress = true;
    else if (value == "STOP") rideInProgress = false;
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

    pub.publish(&wp_msg);
    nh.spinOnce();
    delay(5);
  }
}