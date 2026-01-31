// Auto-generated. Do not edit!

// (in-package custom_msgs.msg)


"use strict";

const _serializer = _ros_msg_utils.Serialize;
const _arraySerializer = _serializer.Array;
const _deserializer = _ros_msg_utils.Deserialize;
const _arrayDeserializer = _deserializer.Array;
const _finder = _ros_msg_utils.Find;
const _getByteLength = _ros_msg_utils.getByteLength;
let std_msgs = _finder('std_msgs');

//-----------------------------------------------------------

class EncoderTicks {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.header = null;
      this.ticks = null;
      this.delta_ticks = null;
    }
    else {
      if (initObj.hasOwnProperty('header')) {
        this.header = initObj.header
      }
      else {
        this.header = new std_msgs.msg.Header();
      }
      if (initObj.hasOwnProperty('ticks')) {
        this.ticks = initObj.ticks
      }
      else {
        this.ticks = 0;
      }
      if (initObj.hasOwnProperty('delta_ticks')) {
        this.delta_ticks = initObj.delta_ticks
      }
      else {
        this.delta_ticks = 0;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type EncoderTicks
    // Serialize message field [header]
    bufferOffset = std_msgs.msg.Header.serialize(obj.header, buffer, bufferOffset);
    // Serialize message field [ticks]
    bufferOffset = _serializer.int32(obj.ticks, buffer, bufferOffset);
    // Serialize message field [delta_ticks]
    bufferOffset = _serializer.int32(obj.delta_ticks, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type EncoderTicks
    let len;
    let data = new EncoderTicks(null);
    // Deserialize message field [header]
    data.header = std_msgs.msg.Header.deserialize(buffer, bufferOffset);
    // Deserialize message field [ticks]
    data.ticks = _deserializer.int32(buffer, bufferOffset);
    // Deserialize message field [delta_ticks]
    data.delta_ticks = _deserializer.int32(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += std_msgs.msg.Header.getMessageSize(object.header);
    return length + 8;
  }

  static datatype() {
    // Returns string type for a message object
    return 'custom_msgs/EncoderTicks';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return 'b02ec1224acd3c659bd318623a2c19a2';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    std_msgs/Header header
    int32 ticks
    int32 delta_ticks
    ================================================================================
    MSG: std_msgs/Header
    # Standard metadata for higher-level stamped data types.
    # This is generally used to communicate timestamped data 
    # in a particular coordinate frame.
    # 
    # sequence ID: consecutively increasing ID 
    uint32 seq
    #Two-integer timestamp that is expressed as:
    # * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')
    # * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')
    # time-handling sugar is provided by the client library
    time stamp
    #Frame this data is associated with
    string frame_id
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new EncoderTicks(null);
    if (msg.header !== undefined) {
      resolved.header = std_msgs.msg.Header.Resolve(msg.header)
    }
    else {
      resolved.header = new std_msgs.msg.Header()
    }

    if (msg.ticks !== undefined) {
      resolved.ticks = msg.ticks;
    }
    else {
      resolved.ticks = 0
    }

    if (msg.delta_ticks !== undefined) {
      resolved.delta_ticks = msg.delta_ticks;
    }
    else {
      resolved.delta_ticks = 0
    }

    return resolved;
    }
};

module.exports = EncoderTicks;
