;; Auto-generated. Do not edit!


(when (boundp 'custom_msgs::Waypoint)
  (if (not (find-package "CUSTOM_MSGS"))
    (make-package "CUSTOM_MSGS"))
  (shadow 'Waypoint (find-package "CUSTOM_MSGS")))
(unless (find-package "CUSTOM_MSGS::WAYPOINT")
  (make-package "CUSTOM_MSGS::WAYPOINT"))

(in-package "ROS")
;;//! \htmlinclude Waypoint.msg.html


(defclass custom_msgs::Waypoint
  :super ros::object
  :slots (_lat _lng ))

(defmethod custom_msgs::Waypoint
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

(setf (get custom_msgs::Waypoint :md5sum-) "321bf96dbd44cdb44c4685a0ded4bea4")
(setf (get custom_msgs::Waypoint :datatype-) "custom_msgs/Waypoint")
(setf (get custom_msgs::Waypoint :definition-)
      "float32 lat
float32 lng
")



(provide :custom_msgs/Waypoint "321bf96dbd44cdb44c4685a0ded4bea4")


