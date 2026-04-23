#include <ros/ros.h>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/PointCloud2.h>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>
#include <pcl/point_types.h>
#include <pcl_ros/point_cloud.h>
#include <pcl_conversions/pcl_conversions.h>
#include <message_filters/subscriber.h>
#include <message_filters/time_synchronizer.h>
#include <message_filters/sync_policies/approximate_time.h>

typedef pcl::PointXYZ PointT;

class StereoToPointCloud {
private:
    ros::NodeHandle nh;
    ros::Publisher stereo_vision_pub;
    
    typedef message_filters::sync_policies::ApproximateTime<sensor_msgs::Image, sensor_msgs::Image> SyncPolicy;
    message_filters::Subscriber<sensor_msgs::Image> cam0_sub; // Right
    message_filters::Subscriber<sensor_msgs::Image> cam1_sub; // Left
    message_filters::Synchronizer<SyncPolicy> sync;

    float fx = 535.4; 
    float fy = 539.2;
    float cx = 320.1; 
    float cy = 247.6;
    float baseline = 0.065; 

    cv::Ptr<cv::StereoBM> stereo;

public:
    StereoToPointCloud() : 
        cam0_sub(nh, "/camera/cam0/image_raw", 1),
        cam1_sub(nh, "/camera/cam1/image_raw", 1),
        sync(SyncPolicy(10), cam1_sub, cam0_sub)
    {
        // ROS Callback binds: _1 is cam1 (Left), _2 is cam0 (Right)
        sync.registerCallback(boost::bind(&StereoToPointCloud::callback, this, _1, _2));
        stereo_vision_pub = nh.advertise<sensor_msgs::PointCloud2>("/stereo_vision/points", 1);
        
        stereo = cv::StereoBM::create(16*5, 21); 
    }

    void callback(const sensor_msgs::ImageConstPtr& cam1_msg, const sensor_msgs::ImageConstPtr& cam0_msg) {
        try {
            cv::Mat cam0_color = cv_bridge::toCvCopy(cam0_msg, "bgr8")->image;
            cv::Mat cam1_color = cv_bridge::toCvCopy(cam1_msg, "bgr8")->image;
            
            cv::Mat cam0_gray, cam1_gray;
            cv::cvtColor(cam0_color, cam0_gray, cv::COLOR_BGR2GRAY);
            cv::cvtColor(cam1_color, cam1_gray, cv::COLOR_BGR2GRAY);

            cv::Mat disparity;
            // OpenCV Logic: Must be (Left, Right). 
            // Since cam1 is Left and cam0 is Right:
            stereo->compute(cam1_gray, cam0_gray, disparity);

            pcl::PointCloud<PointT>::Ptr cloud(new pcl::PointCloud<PointT>);
            cloud->header.frame_id = cam1_msg->header.frame_id;
            
            cloud->width = std::ceil(disparity.cols / 2.0);
            cloud->height = std::ceil(disparity.rows / 2.0);
            cloud->is_dense = false;
            cloud->points.resize(cloud->width * cloud->height);

            for (int v = 0; v < disparity.rows; v += 2) {
                for (int u = 0; u < disparity.cols; u += 2) {
                    float disp = disparity.at<short>(v, u) / 16.0f;
                    
                    int new_u = u / 2;
                    int new_v = v / 2;
                    int index = new_v * cloud->width + new_u;

                    if (disp <= 0) {
                        cloud->points[index].x = std::numeric_limits<float>::quiet_NaN();
                        cloud->points[index].y = std::numeric_limits<float>::quiet_NaN();
                        cloud->points[index].z = std::numeric_limits<float>::quiet_NaN();
                    } else {
                        float z = (fx * baseline) / disp;
                        
                        if (z > 0.5 && z < 15.0) {
                            cloud->points[index].z = z;
                            cloud->points[index].x = (u - cx) * z / fx;
                            cloud->points[index].y = (v - cy) * z / fy;
                        } else {
                            cloud->points[index].x = std::numeric_limits<float>::quiet_NaN();
                            cloud->points[index].y = std::numeric_limits<float>::quiet_NaN();
                            cloud->points[index].z = std::numeric_limits<float>::quiet_NaN();
                        }
                    }
                }
            }

            if (!cloud->empty()) {
                sensor_msgs::PointCloud2 output;
                pcl::toROSMsg(*cloud, output);
                output.header = cam1_msg->header;
                stereo_vision_pub.publish(output);
            }
        }
        catch (cv_bridge::Exception& e) {
            ROS_ERROR("cv_bridge exception: %s", e.what());
        }
    }
};

int main(int argc, char** argv) {
    ros::init(argc, argv, "stereo_vision_node");
    StereoToPointCloud sp;
    ros::spin();
    return 0;
}