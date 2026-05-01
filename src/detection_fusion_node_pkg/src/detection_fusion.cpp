#include <ros/ros.h>
#include <message_filters/subscriber.h>
#include <message_filters/sync_policies/approximate_time.h>
#include <message_filters/synchronizer.h>
#include <vision_msgs/Detection2DArray.h>
#include <sensor_msgs/PointCloud2.h>
#include <pcl_ros/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl_conversions/pcl_conversions.h>
#include <vector>
#include <algorithm>

#include <custom_msgs/DetectionDistance.h>
#include <custom_msgs/DetectionDistanceArray.h>

typedef pcl::PointXYZ PointT;
typedef message_filters::sync_policies::ApproximateTime<vision_msgs::Detection2DArray, sensor_msgs::PointCloud2> SyncPolicy;

ros::Publisher pub_obstacles;

void fusionCallback(const vision_msgs::Detection2DArrayConstPtr& yolo_msg, const sensor_msgs::PointCloud2ConstPtr& cloud_msg) {
    pcl::PointCloud<PointT>::Ptr cloud(new pcl::PointCloud<PointT>);
    pcl::fromROSMsg(*cloud_msg, *cloud);

    // Skip if point cloud is missing or YOLO found nothing this frame
    if (cloud->empty() || yolo_msg->detections.empty()) return;

    custom_msgs::DetectionDistanceArray out_array;
    out_array.header = yolo_msg->header;

    for (const auto& det : yolo_msg->detections) {
        if (det.results.empty()) continue;

        int class_id = det.results[0].id;
        
        // 1. Direct 1:1 mapping from YOLO bounding box to Point Cloud pixels
        int min_x = std::max(0, (int)(det.bbox.center.x - det.bbox.size_x / 2.0f));
        int max_x = std::min((int)cloud->width - 1, (int)(det.bbox.center.x + det.bbox.size_x / 2.0f));
        int min_y = std::max(0, (int)(det.bbox.center.y - det.bbox.size_y / 2.0f));
        int max_y = std::min((int)cloud->height - 1, (int)(det.bbox.center.y + det.bbox.size_y / 2.0f));

        std::vector<float> valid_distances;
        // Optimization: Pre-allocate memory to prevent slow vector resizing inside the loop
        valid_distances.reserve((max_x - min_x) * (max_y - min_y)); 

        // 2. Extract valid depth (Z) values within the bounding box
        for (int y = min_y; y <= max_y; ++y) {
            for (int x = min_x; x <= max_x; ++x) {
                int index = y * cloud->width + x;
                
                if (index < cloud->points.size()) {
                    float z = cloud->points[index].z;
                    // Ignore NaN, Inf, or negative depths
                    if (std::isfinite(z) && z > 0.0f) {
                        valid_distances.push_back(z);
                    }
                }
            }
        }

        // 3. Calculate median distance to ignore outliers (like background noise)
        if (!valid_distances.empty()) {
            std::sort(valid_distances.begin(), valid_distances.end());
            float median_distance = valid_distances[valid_distances.size() / 2];

            custom_msgs::DetectionDistance dist_msg;
            dist_msg.class_id = class_id;
            dist_msg.distance = median_distance;
            out_array.detections.push_back(dist_msg);
        }
    }

    pub_obstacles.publish(out_array);
}

int main(int argc, char** argv) {
    ros::init(argc, argv, "detection_fusion_node");
    ros::NodeHandle nh;

    pub_obstacles = nh.advertise<custom_msgs::DetectionDistanceArray>("/perception/obstacles", 1);

    // Subscribing to the YOLO detections and the new stereo_image_proc point cloud
    message_filters::Subscriber<vision_msgs::Detection2DArray> sub_yolo(nh, "/vision/detections", 1);
    message_filters::Subscriber<sensor_msgs::PointCloud2> sub_cloud(nh, "/camera/points2", 1);

    // ApproximateTime syncs the YOLO frame and Point Cloud frame by their closest timestamps
    message_filters::Synchronizer<SyncPolicy> sync(SyncPolicy(10), sub_yolo, sub_cloud);
    sync.registerCallback(boost::bind(&fusionCallback, _1, _2));

    ROS_INFO("Fusion Node Active. Publishing distances to /perception/obstacles");

    ros::spin();
    return 0;
}