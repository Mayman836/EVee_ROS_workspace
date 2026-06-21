#include <ros/ros.h>
#include <custom_msgs/Waypoint.h>
#include <std_msgs/Bool.h>
#include <vector>

class WaypointManager {
private:
    ros::NodeHandle nh_;
    ros::Subscriber wp_sub_;
    ros::Subscriber reached_sub_;
    ros::Publisher target_pub_;
    ros::Timer pub_timer_;

    std::vector<custom_msgs::Waypoint> waypoint_list_;
    size_t current_wp_index_;
    bool execution_active_;

public:
    WaypointManager() : current_wp_index_(0), execution_active_(false) {
        wp_sub_ = nh_.subscribe("/navigation/goal", 100, &WaypointManager::waypointCallback, this);
        reached_sub_ = nh_.subscribe("/navigation/waypoint_reached", 10, &WaypointManager::reachedCallback, this);
        target_pub_ = nh_.advertise<custom_msgs::Waypoint>("/navigation/current_target", 10);

        // Create a timer that fires every 0.1 seconds (10 Hz) to continuously publish
        pub_timer_ = nh_.createTimer(ros::Duration(0.1), &WaypointManager::timerCallback, this);

        ROS_INFO("Waypoint Manager Node Initialized.");
    }

    void waypointCallback(const custom_msgs::Waypoint& msg) {
        waypoint_list_.push_back(msg);
        ROS_INFO("Received and stored waypoint: Lat: %f, Lng: %f. Total stored: %lu", 
                 msg.lat, msg.lng, waypoint_list_.size());

        if (!execution_active_ && waypoint_list_.size() == 1) {
            execution_active_ = true;
            current_wp_index_ = 0;
            ROS_INFO("Starting execution towards Waypoint 1!");
        }
    }

    void reachedCallback(const std_msgs::Bool::ConstPtr& msg) {
        if (msg->data && execution_active_) {
            ROS_INFO("Waypoint %lu reached!", current_wp_index_ + 1);
            
            current_wp_index_++;
            
            if (current_wp_index_ < waypoint_list_.size()) {
                ROS_INFO("Switching to Waypoint %lu...", current_wp_index_ + 1);
            } else {
                ROS_INFO("All waypoints completed! Clearing buffer.");
                waypoint_list_.clear();
                current_wp_index_ = 0;
                execution_active_ = false;
            }
        }
    }

    void timerCallback(const ros::TimerEvent& event) {
        // Continuously publish as long as execution is active
        if (execution_active_ && current_wp_index_ < waypoint_list_.size()) {
            target_pub_.publish(waypoint_list_[current_wp_index_]);

            // Print to the terminal once every 1.0 seconds
            ROS_INFO_THROTTLE(1.0, "Continuously publishing Waypoint %lu: Lat: %f, Lng: %f", 
                              current_wp_index_ + 1, 
                              waypoint_list_[current_wp_index_].lat, 
                              waypoint_list_[current_wp_index_].lng);
        }
    }
};

int main(int argc, char** argv) {
    ros::init(argc, argv, "waypoint_manager_node");
    WaypointManager wm;
    
    // ros::spin() now handles both incoming messages AND our publishing timer
    ros::spin(); 
    return 0;
}