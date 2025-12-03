// Auto-generated. Do not edit!

// (in-package custom_msgs.msg)


"use strict";

const _serializer = _ros_msg_utils.Serialize;
const _arraySerializer = _serializer.Array;
const _deserializer = _ros_msg_utils.Deserialize;
const _arrayDeserializer = _deserializer.Array;
const _finder = _ros_msg_utils.Find;
const _getByteLength = _ros_msg_utils.getByteLength;

//-----------------------------------------------------------

class Waypoint {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.lat = null;
      this.lng = null;
    }
    else {
      if (initObj.hasOwnProperty('lat')) {
        this.lat = initObj.lat
      }
      else {
        this.lat = 0.0;
      }
      if (initObj.hasOwnProperty('lng')) {
        this.lng = initObj.lng
      }
      else {
        this.lng = 0.0;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type Waypoint
    // Serialize message field [lat]
    bufferOffset = _serializer.float32(obj.lat, buffer, bufferOffset);
    // Serialize message field [lng]
    bufferOffset = _serializer.float32(obj.lng, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type Waypoint
    let len;
    let data = new Waypoint(null);
    // Deserialize message field [lat]
    data.lat = _deserializer.float32(buffer, bufferOffset);
    // Deserialize message field [lng]
    data.lng = _deserializer.float32(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    return 8;
  }

  static datatype() {
    // Returns string type for a message object
    return 'custom_msgs/Waypoint';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '321bf96dbd44cdb44c4685a0ded4bea4';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    float32 lat
    float32 lng
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new Waypoint(null);
    if (msg.lat !== undefined) {
      resolved.lat = msg.lat;
    }
    else {
      resolved.lat = 0.0
    }

    if (msg.lng !== undefined) {
      resolved.lng = msg.lng;
    }
    else {
      resolved.lng = 0.0
    }

    return resolved;
    }
};

module.exports = Waypoint;
