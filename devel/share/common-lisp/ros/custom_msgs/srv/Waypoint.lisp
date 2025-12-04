; Auto-generated. Do not edit!


(cl:in-package custom_msgs-srv)


;//! \htmlinclude Waypoint-request.msg.html

(cl:defclass <Waypoint-request> (roslisp-msg-protocol:ros-message)
  ((index
    :reader index
    :initarg :index
    :type cl:integer
    :initform 0))
)

(cl:defclass Waypoint-request (<Waypoint-request>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <Waypoint-request>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'Waypoint-request)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name custom_msgs-srv:<Waypoint-request> is deprecated: use custom_msgs-srv:Waypoint-request instead.")))

(cl:ensure-generic-function 'index-val :lambda-list '(m))
(cl:defmethod index-val ((m <Waypoint-request>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader custom_msgs-srv:index-val is deprecated.  Use custom_msgs-srv:index instead.")
  (index m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <Waypoint-request>) ostream)
  "Serializes a message object of type '<Waypoint-request>"
  (cl:let* ((signed (cl:slot-value msg 'index)) (unsigned (cl:if (cl:< signed 0) (cl:+ signed 4294967296) signed)))
    (cl:write-byte (cl:ldb (cl:byte 8 0) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) unsigned) ostream)
    )
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <Waypoint-request>) istream)
  "Deserializes a message object of type '<Waypoint-request>"
    (cl:let ((unsigned 0))
      (cl:setf (cl:ldb (cl:byte 8 0) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) unsigned) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'index) (cl:if (cl:< unsigned 2147483648) unsigned (cl:- unsigned 4294967296))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<Waypoint-request>)))
  "Returns string type for a service object of type '<Waypoint-request>"
  "custom_msgs/WaypointRequest")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'Waypoint-request)))
  "Returns string type for a service object of type 'Waypoint-request"
  "custom_msgs/WaypointRequest")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<Waypoint-request>)))
  "Returns md5sum for a message object of type '<Waypoint-request>"
  "4b08d424d393f42d8d9403171e1b74c8")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'Waypoint-request)))
  "Returns md5sum for a message object of type 'Waypoint-request"
  "4b08d424d393f42d8d9403171e1b74c8")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<Waypoint-request>)))
  "Returns full string definition for message of type '<Waypoint-request>"
  (cl:format cl:nil "int32 index~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'Waypoint-request)))
  "Returns full string definition for message of type 'Waypoint-request"
  (cl:format cl:nil "int32 index~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <Waypoint-request>))
  (cl:+ 0
     4
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <Waypoint-request>))
  "Converts a ROS message object to a list"
  (cl:list 'Waypoint-request
    (cl:cons ':index (index msg))
))
;//! \htmlinclude Waypoint-response.msg.html

(cl:defclass <Waypoint-response> (roslisp-msg-protocol:ros-message)
  ((lat
    :reader lat
    :initarg :lat
    :type cl:float
    :initform 0.0)
   (lng
    :reader lng
    :initarg :lng
    :type cl:float
    :initform 0.0))
)

(cl:defclass Waypoint-response (<Waypoint-response>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <Waypoint-response>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'Waypoint-response)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name custom_msgs-srv:<Waypoint-response> is deprecated: use custom_msgs-srv:Waypoint-response instead.")))

(cl:ensure-generic-function 'lat-val :lambda-list '(m))
(cl:defmethod lat-val ((m <Waypoint-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader custom_msgs-srv:lat-val is deprecated.  Use custom_msgs-srv:lat instead.")
  (lat m))

(cl:ensure-generic-function 'lng-val :lambda-list '(m))
(cl:defmethod lng-val ((m <Waypoint-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader custom_msgs-srv:lng-val is deprecated.  Use custom_msgs-srv:lng instead.")
  (lng m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <Waypoint-response>) ostream)
  "Serializes a message object of type '<Waypoint-response>"
  (cl:let ((bits (roslisp-utils:encode-single-float-bits (cl:slot-value msg 'lat))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream))
  (cl:let ((bits (roslisp-utils:encode-single-float-bits (cl:slot-value msg 'lng))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <Waypoint-response>) istream)
  "Deserializes a message object of type '<Waypoint-response>"
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'lat) (roslisp-utils:decode-single-float-bits bits)))
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'lng) (roslisp-utils:decode-single-float-bits bits)))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<Waypoint-response>)))
  "Returns string type for a service object of type '<Waypoint-response>"
  "custom_msgs/WaypointResponse")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'Waypoint-response)))
  "Returns string type for a service object of type 'Waypoint-response"
  "custom_msgs/WaypointResponse")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<Waypoint-response>)))
  "Returns md5sum for a message object of type '<Waypoint-response>"
  "4b08d424d393f42d8d9403171e1b74c8")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'Waypoint-response)))
  "Returns md5sum for a message object of type 'Waypoint-response"
  "4b08d424d393f42d8d9403171e1b74c8")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<Waypoint-response>)))
  "Returns full string definition for message of type '<Waypoint-response>"
  (cl:format cl:nil "float32 lat~%float32 lng~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'Waypoint-response)))
  "Returns full string definition for message of type 'Waypoint-response"
  (cl:format cl:nil "float32 lat~%float32 lng~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <Waypoint-response>))
  (cl:+ 0
     4
     4
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <Waypoint-response>))
  "Converts a ROS message object to a list"
  (cl:list 'Waypoint-response
    (cl:cons ':lat (lat msg))
    (cl:cons ':lng (lng msg))
))
(cl:defmethod roslisp-msg-protocol:service-request-type ((msg (cl:eql 'Waypoint)))
  'Waypoint-request)
(cl:defmethod roslisp-msg-protocol:service-response-type ((msg (cl:eql 'Waypoint)))
  'Waypoint-response)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'Waypoint)))
  "Returns string type for a service object of type '<Waypoint>"
  "custom_msgs/Waypoint")