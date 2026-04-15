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

using namespace sensor_msgs;
using namespace vision_msgs;
using namespace message_filters;

typedef pcl::PointXYZ PointT; 
typedef sync_policies::ApproximateTime<Detection2DArray, PointCloud2> SyncPolicy;

std::vector<std::string> class_names = {
    "human", "speed_bump", "vehicle"
};

void detectionFusionCallback(const Detection2DArrayConstPtr& yolo_msg, const PointCloud2ConstPtr& stereo_vision_msg)
{
    pcl::PointCloud<PointT>::Ptr cloud(new pcl::PointCloud<PointT>);
    pcl::fromROSMsg(*stereo_vision_msg, *cloud);

    if (cloud->empty()) return;

    for (size_t i = 0; i < yolo_msg->detections.size(); i++) {
        const auto& det = yolo_msg->detections[i];
        
        int class_id = det.results[0].id;
        std::string obj_name = (class_id >= 0 && class_id < class_names.size()) ? class_names[class_id] : "unknown";

        float center_x = det.bbox.center.x;
        float center_y = det.bbox.center.y;
        float size_x = det.bbox.size_x;
        float size_y = det.bbox.size_y;

        int yolo_min_x = std::max(0, (int)(center_x - size_x / 2));
        int yolo_max_x = (int)(center_x + size_x / 2);
        int yolo_min_y = std::max(0, (int)(center_y - size_y / 2));
        int yolo_max_y = (int)(center_y + size_y / 2);

        int cloud_min_x = yolo_min_x / 2;
        int cloud_max_x = yolo_max_x / 2;
        int cloud_min_y = yolo_min_y / 2;
        int cloud_max_y = yolo_max_y / 2;

        cloud_max_x = std::min(cloud_max_x, (int)cloud->width - 1);
        cloud_max_y = std::min(cloud_max_y, (int)cloud->height - 1);

        std::vector<float> z_distances;

        for (int y = cloud_min_y; y <= cloud_max_y; y++) {
            for (int x = cloud_min_x; x <= cloud_max_x; x++) {
                int index = y * cloud->width + x;
                
                if (index < cloud->points.size()) {
                    float z = cloud->points[index].z;
                    if (std::isfinite(z) && z > 0) {
                        z_distances.push_back(z);
                    }
                }
            }
        }

        if (!z_distances.empty()) {
            std::sort(z_distances.begin(), z_distances.end());
            float final_distance = z_distances[z_distances.size() / 2];
            ROS_INFO("%s %.2f meters", obj_name.c_str(), final_distance);
        } else {
            ROS_INFO("%s distance unknown", obj_name.c_str());
        }
    }
    ROS_INFO("-------------------------");
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "detection_fusion_node");
    ros::NodeHandle nh;

    message_filters::Subscriber<Detection2DArray> yolo_sub(nh, "/vision/detections", 10);
    message_filters::Subscriber<PointCloud2> stereo_vision_sub(nh, "/stereo_vision/points", 10);

    Synchronizer<SyncPolicy> sync(SyncPolicy(10), yolo_sub, stereo_vision_sub);
    sync.registerCallback(boost::bind(&detectionFusionCallback, _1, _2));

    ROS_INFO("Fusion Node Started. Waiting for data...");

    ros::spin();
    return 0;
}