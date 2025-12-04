;; Auto-generated. Do not edit!


(when (boundp 'custom_msgs::Waypoint)
  (if (not (find-package "CUSTOM_MSGS"))
    (make-package "CUSTOM_MSGS"))
  (shadow 'Waypoint (find-package "CUSTOM_MSGS")))
(unless (find-package "CUSTOM_MSGS::WAYPOINT")
  (make-package "CUSTOM_MSGS::WAYPOINT"))
(unless (find-package "CUSTOM_MSGS::WAYPOINTREQUEST")
  (make-package "CUSTOM_MSGS::WAYPOINTREQUEST"))
(unless (find-package "CUSTOM_MSGS::WAYPOINTRESPONSE")
  (make-package "CUSTOM_MSGS::WAYPOINTRESPONSE"))

(in-package "ROS")





(defclass custom_msgs::WaypointRequest
  :super ros::object
  :slots (_index ))

(defmethod custom_msgs::WaypointRequest
  (:init
   (&key
    ((:index __index) 0)
    )
   (send-super :init)
   (setq _index (round __index))
   self)
  (:index
   (&optional __index)
   (if __index (setq _index __index)) _index)
  (:serialization-length
   ()
   (+
    ;; int32 _index
    4
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; int32 _index
       (write-long _index s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; int32 _index
     (setq _index (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;;
   self)
  )

(defclass custom_msgs::WaypointResponse
  :super ros::object
  :slots (_lat _lng ))

(defmethod custom_msgs::WaypointResponse
  (:init
   (&key
    ((:lat __lat) 0.0)
    ((:lng __lng) 0.0)
    )
   (send-super :init)
   (setq _lat (float __lat))
   (setq _lng (float __lng))
   self)
  (:lat
   (&optional __lat)
   (if __lat (setq _lat __lat)) _lat)
  (:lng
   (&optional __lng)
   (if __lng (setq _lng __lng)) _lng)
  (:serialization-length
   ()
   (+
    ;; float32 _lat
    4
    ;; float32 _lng
    4
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; float32 _lat
       (sys::poke _lat (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; float32 _lng
       (sys::poke _lng (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; float32 _lat
     (setq _lat (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; float32 _lng
     (setq _lng (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;;
   self)
  )

(defclass custom_msgs::Waypoint
  :super ros::object
  :slots ())

(setf (get custom_msgs::Waypoint :md5sum-) "4b08d424d393f42d8d9403171e1b74c8")
(setf (get custom_msgs::Waypoint :datatype-) "custom_msgs/Waypoint")
(setf (get custom_msgs::Waypoint :request) custom_msgs::WaypointRequest)
(setf (get custom_msgs::Waypoint :response) custom_msgs::WaypointResponse)

(defmethod custom_msgs::WaypointRequest
  (:response () (instance custom_msgs::WaypointResponse :init)))

(setf (get custom_msgs::WaypointRequest :md5sum-) "4b08d424d393f42d8d9403171e1b74c8")
(setf (get custom_msgs::WaypointRequest :datatype-) "custom_msgs/WaypointRequest")
(setf (get custom_msgs::WaypointRequest :definition-)
      "int32 index
---
float32 lat
float32 lng
")

(setf (get custom_msgs::WaypointResponse :md5sum-) "4b08d424d393f42d8d9403171e1b74c8")
(setf (get custom_msgs::WaypointResponse :datatype-) "custom_msgs/WaypointResponse")
(setf (get custom_msgs::WaypointResponse :definition-)
      "int32 index
---
float32 lat
float32 lng
")



(provide :custom_msgs/Waypoint "4b08d424d393f42d8d9403171e1b74c8")


