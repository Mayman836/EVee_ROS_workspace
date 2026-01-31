;; Auto-generated. Do not edit!


(when (boundp 'custom_msgs::EncoderTicks)
  (if (not (find-package "CUSTOM_MSGS"))
    (make-package "CUSTOM_MSGS"))
  (shadow 'EncoderTicks (find-package "CUSTOM_MSGS")))
(unless (find-package "CUSTOM_MSGS::ENCODERTICKS")
  (make-package "CUSTOM_MSGS::ENCODERTICKS"))

(in-package "ROS")
;;//! \htmlinclude EncoderTicks.msg.html
(if (not (find-package "STD_MSGS"))
  (ros::roseus-add-msgs "std_msgs"))


(defclass custom_msgs::EncoderTicks
  :super ros::object
  :slots (_header _ticks _delta_ticks ))

(defmethod custom_msgs::EncoderTicks
  (:init
   (&key
    ((:header __header) (instance std_msgs::Header :init))
    ((:ticks __ticks) 0)
    ((:delta_ticks __delta_ticks) 0)
    )
   (send-super :init)
   (setq _header __header)
   (setq _ticks (round __ticks))
   (setq _delta_ticks (round __delta_ticks))
   self)
  (:header
   (&rest __header)
   (if (keywordp (car __header))
       (send* _header __header)
     (progn
       (if __header (setq _header (car __header)))
       _header)))
  (:ticks
   (&optional __ticks)
   (if __ticks (setq _ticks __ticks)) _ticks)
  (:delta_ticks
   (&optional __delta_ticks)
   (if __delta_ticks (setq _delta_ticks __delta_ticks)) _delta_ticks)
  (:serialization-length
   ()
   (+
    ;; std_msgs/Header _header
    (send _header :serialization-length)
    ;; int32 _ticks
    4
    ;; int32 _delta_ticks
    4
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; std_msgs/Header _header
       (send _header :serialize s)
     ;; int32 _ticks
       (write-long _ticks s)
     ;; int32 _delta_ticks
       (write-long _delta_ticks s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; std_msgs/Header _header
     (send _header :deserialize buf ptr-) (incf ptr- (send _header :serialization-length))
   ;; int32 _ticks
     (setq _ticks (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; int32 _delta_ticks
     (setq _delta_ticks (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;;
   self)
  )

(setf (get custom_msgs::EncoderTicks :md5sum-) "b02ec1224acd3c659bd318623a2c19a2")
(setf (get custom_msgs::EncoderTicks :datatype-) "custom_msgs/EncoderTicks")
(setf (get custom_msgs::EncoderTicks :definition-)
      "std_msgs/Header header
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

")



(provide :custom_msgs/EncoderTicks "b02ec1224acd3c659bd318623a2c19a2")


