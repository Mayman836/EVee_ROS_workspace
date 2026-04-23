;; Auto-generated. Do not edit!


(when (boundp 'custom_msgs::DetectionDistance)
  (if (not (find-package "CUSTOM_MSGS"))
    (make-package "CUSTOM_MSGS"))
  (shadow 'DetectionDistance (find-package "CUSTOM_MSGS")))
(unless (find-package "CUSTOM_MSGS::DETECTIONDISTANCE")
  (make-package "CUSTOM_MSGS::DETECTIONDISTANCE"))

(in-package "ROS")
;;//! \htmlinclude DetectionDistance.msg.html


(defclass custom_msgs::DetectionDistance
  :super ros::object
  :slots (_class_id _distance ))

(defmethod custom_msgs::DetectionDistance
  (:init
   (&key
    ((:class_id __class_id) 0)
    ((:distance __distance) 0.0)
    )
   (send-super :init)
   (setq _class_id (round __class_id))
   (setq _distance (float __distance))
   self)
  (:class_id
   (&optional __class_id)
   (if __class_id (setq _class_id __class_id)) _class_id)
  (:distance
   (&optional __distance)
   (if __distance (setq _distance __distance)) _distance)
  (:serialization-length
   ()
   (+
    ;; int32 _class_id
    4
    ;; float32 _distance
    4
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; int32 _class_id
       (write-long _class_id s)
     ;; float32 _distance
       (sys::poke _distance (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; int32 _class_id
     (setq _class_id (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; float32 _distance
     (setq _distance (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;;
   self)
  )

(setf (get custom_msgs::DetectionDistance :md5sum-) "175cb11d2b584f9edc572f3c2d08635a")
(setf (get custom_msgs::DetectionDistance :datatype-) "custom_msgs/DetectionDistance")
(setf (get custom_msgs::DetectionDistance :definition-)
      "int32 class_id
float32 distance
")



(provide :custom_msgs/DetectionDistance "175cb11d2b584f9edc572f3c2d08635a")


