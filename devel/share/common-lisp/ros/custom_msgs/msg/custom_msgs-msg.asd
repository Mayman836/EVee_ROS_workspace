
(cl:in-package :asdf)

(defsystem "custom_msgs-msg"
  :depends-on (:roslisp-msg-protocol :roslisp-utils :std_msgs-msg
)
  :components ((:file "_package")
    (:file "DetectionDistance" :depends-on ("_package_DetectionDistance"))
    (:file "_package_DetectionDistance" :depends-on ("_package"))
    (:file "DetectionDistanceArray" :depends-on ("_package_DetectionDistanceArray"))
    (:file "_package_DetectionDistanceArray" :depends-on ("_package"))
    (:file "EncoderTicks" :depends-on ("_package_EncoderTicks"))
    (:file "_package_EncoderTicks" :depends-on ("_package"))
    (:file "Waypoint" :depends-on ("_package_Waypoint"))
    (:file "_package_Waypoint" :depends-on ("_package"))
  ))