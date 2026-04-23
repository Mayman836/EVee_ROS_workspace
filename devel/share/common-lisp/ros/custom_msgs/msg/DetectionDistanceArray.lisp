; Auto-generated. Do not edit!


(cl:in-package custom_msgs-msg)


;//! \htmlinclude DetectionDistanceArray.msg.html

(cl:defclass <DetectionDistanceArray> (roslisp-msg-protocol:ros-message)
  ((header
    :reader header
    :initarg :header
    :type std_msgs-msg:Header
    :initform (cl:make-instance 'std_msgs-msg:Header))
   (detections
    :reader detections
    :initarg :detections
    :type (cl:vector custom_msgs-msg:DetectionDistance)
   :initform (cl:make-array 0 :element-type 'custom_msgs-msg:DetectionDistance :initial-element (cl:make-instance 'custom_msgs-msg:DetectionDistance))))
)

(cl:defclass DetectionDistanceArray (<DetectionDistanceArray>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <DetectionDistanceArray>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'DetectionDistanceArray)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name custom_msgs-msg:<DetectionDistanceArray> is deprecated: use custom_msgs-msg:DetectionDistanceArray instead.")))

(cl:ensure-generic-function 'header-val :lambda-list '(m))
(cl:defmethod header-val ((m <DetectionDistanceArray>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader custom_msgs-msg:header-val is deprecated.  Use custom_msgs-msg:header instead.")
  (header m))

(cl:ensure-generic-function 'detections-val :lambda-list '(m))
(cl:defmethod detections-val ((m <DetectionDistanceArray>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader custom_msgs-msg:detections-val is deprecated.  Use custom_msgs-msg:detections instead.")
  (detections m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <DetectionDistanceArray>) ostream)
  "Serializes a message object of type '<DetectionDistanceArray>"
  (roslisp-msg-protocol:serialize (cl:slot-value msg 'header) ostream)
  (cl:let ((__ros_arr_len (cl:length (cl:slot-value msg 'detections))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_arr_len) ostream))
  (cl:map cl:nil #'(cl:lambda (ele) (roslisp-msg-protocol:serialize ele ostream))
   (cl:slot-value msg 'detections))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <DetectionDistanceArray>) istream)
  "Deserializes a message object of type '<DetectionDistanceArray>"
  (roslisp-msg-protocol:deserialize (cl:slot-value msg 'header) istream)
  (cl:let ((__ros_arr_len 0))
    (cl:setf (cl:ldb (cl:byte 8 0) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) __ros_arr_len) (cl:read-byte istream))
  (cl:setf (cl:slot-value msg 'detections) (cl:make-array __ros_arr_len))
  (cl:let ((vals (cl:slot-value msg 'detections)))
    (cl:dotimes (i __ros_arr_len)
    (cl:setf (cl:aref vals i) (cl:make-instance 'custom_msgs-msg:DetectionDistance))
  (roslisp-msg-protocol:deserialize (cl:aref vals i) istream))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<DetectionDistanceArray>)))
  "Returns string type for a message object of type '<DetectionDistanceArray>"
  "custom_msgs/DetectionDistanceArray")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'DetectionDistanceArray)))
  "Returns string type for a message object of type 'DetectionDistanceArray"
  "custom_msgs/DetectionDistanceArray")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<DetectionDistanceArray>)))
  "Returns md5sum for a message object of type '<DetectionDistanceArray>"
  "c3f1012552fe33467f794d026ae1595f")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'DetectionDistanceArray)))
  "Returns md5sum for a message object of type 'DetectionDistanceArray"
  "c3f1012552fe33467f794d026ae1595f")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<DetectionDistanceArray>)))
  "Returns full string definition for message of type '<DetectionDistanceArray>"
  (cl:format cl:nil "std_msgs/Header header~%DetectionDistance[] detections~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%string frame_id~%~%================================================================================~%MSG: custom_msgs/DetectionDistance~%int32 class_id~%float32 distance~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'DetectionDistanceArray)))
  "Returns full string definition for message of type 'DetectionDistanceArray"
  (cl:format cl:nil "std_msgs/Header header~%DetectionDistance[] detections~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%string frame_id~%~%================================================================================~%MSG: custom_msgs/DetectionDistance~%int32 class_id~%float32 distance~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <DetectionDistanceArray>))
  (cl:+ 0
     (roslisp-msg-protocol:serialization-length (cl:slot-value msg 'header))
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'detections) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ (roslisp-msg-protocol:serialization-length ele))))
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <DetectionDistanceArray>))
  "Converts a ROS message object to a list"
  (cl:list 'DetectionDistanceArray
    (cl:cons ':header (header msg))
    (cl:cons ':detections (detections msg))
))
