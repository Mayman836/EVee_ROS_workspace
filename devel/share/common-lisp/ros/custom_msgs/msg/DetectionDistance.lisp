; Auto-generated. Do not edit!


(cl:in-package custom_msgs-msg)


;//! \htmlinclude DetectionDistance.msg.html

(cl:defclass <DetectionDistance> (roslisp-msg-protocol:ros-message)
  ((class_id
    :reader class_id
    :initarg :class_id
    :type cl:integer
    :initform 0)
   (distance
    :reader distance
    :initarg :distance
    :type cl:float
    :initform 0.0))
)

(cl:defclass DetectionDistance (<DetectionDistance>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <DetectionDistance>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'DetectionDistance)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name custom_msgs-msg:<DetectionDistance> is deprecated: use custom_msgs-msg:DetectionDistance instead.")))

(cl:ensure-generic-function 'class_id-val :lambda-list '(m))
(cl:defmethod class_id-val ((m <DetectionDistance>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader custom_msgs-msg:class_id-val is deprecated.  Use custom_msgs-msg:class_id instead.")
  (class_id m))

(cl:ensure-generic-function 'distance-val :lambda-list '(m))
(cl:defmethod distance-val ((m <DetectionDistance>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader custom_msgs-msg:distance-val is deprecated.  Use custom_msgs-msg:distance instead.")
  (distance m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <DetectionDistance>) ostream)
  "Serializes a message object of type '<DetectionDistance>"
  (cl:let* ((signed (cl:slot-value msg 'class_id)) (unsigned (cl:if (cl:< signed 0) (cl:+ signed 4294967296) signed)))
    (cl:write-byte (cl:ldb (cl:byte 8 0) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) unsigned) ostream)
    )
  (cl:let ((bits (roslisp-utils:encode-single-float-bits (cl:slot-value msg 'distance))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <DetectionDistance>) istream)
  "Deserializes a message object of type '<DetectionDistance>"
    (cl:let ((unsigned 0))
      (cl:setf (cl:ldb (cl:byte 8 0) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) unsigned) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'class_id) (cl:if (cl:< unsigned 2147483648) unsigned (cl:- unsigned 4294967296))))
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'distance) (roslisp-utils:decode-single-float-bits bits)))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<DetectionDistance>)))
  "Returns string type for a message object of type '<DetectionDistance>"
  "custom_msgs/DetectionDistance")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'DetectionDistance)))
  "Returns string type for a message object of type 'DetectionDistance"
  "custom_msgs/DetectionDistance")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<DetectionDistance>)))
  "Returns md5sum for a message object of type '<DetectionDistance>"
  "175cb11d2b584f9edc572f3c2d08635a")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'DetectionDistance)))
  "Returns md5sum for a message object of type 'DetectionDistance"
  "175cb11d2b584f9edc572f3c2d08635a")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<DetectionDistance>)))
  "Returns full string definition for message of type '<DetectionDistance>"
  (cl:format cl:nil "int32 class_id~%float32 distance~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'DetectionDistance)))
  "Returns full string definition for message of type 'DetectionDistance"
  (cl:format cl:nil "int32 class_id~%float32 distance~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <DetectionDistance>))
  (cl:+ 0
     4
     4
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <DetectionDistance>))
  "Converts a ROS message object to a list"
  (cl:list 'DetectionDistance
    (cl:cons ':class_id (class_id msg))
    (cl:cons ':distance (distance msg))
))
