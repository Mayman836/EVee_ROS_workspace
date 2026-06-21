#include <ros/ros.h>
#include <eigen3/Eigen/Dense>
#include <tf/transform_datatypes.h>
#include <sensor_msgs/Imu.h>
#include <sensor_msgs/NavSatFix.h>
#include <nav_msgs/Odometry.h>
#include <custom_msgs/EncoderTicks.h>
#include <std_msgs/Float64.h>

const double R_earth = 6378137.0;

ros::Publisher ekf_pub;
ros::Subscriber l_hall_sub;
ros::Subscriber r_hall_sub;
ros::Subscriber s_enc_sub;
ros::Subscriber imu_sub;
ros::Subscriber gps_sub;

sensor_msgs::Imu imu_msg;
sensor_msgs::NavSatFix gps_msg;

bool steering_received = false;
bool imu_received = false;
bool ekf_initialized = false;
bool gps_initialized = false;

ros::Time last_l_hall_time;
ros::Time last_r_hall_time;
ros::Time last_imu_time;
ros::Time last_gps_time;
ros::Time last_used_imu_time;
ros::Time last_used_gps_time;

double wheel_radius = 0.127;
double wheelbase = 0.805;
double max_steer = 45 * M_PI / 180.0;

// Hall sensor tracking variables
double l_hall_w = 0.0;
double r_hall_w = 0.0;
int32_t last_l_hall_ticks = 0;
int32_t last_r_hall_ticks = 0;
bool first_l_hall = true;
bool first_r_hall = true;

// Steering tracking
double s_enc_angle = 0.0;

// GPS tracking
double lat0 = 0.0;
double lng0 = 0.0;
double x_meas;
double y_meas;

Eigen::VectorXd X(5);
Eigen::MatrixXd P = Eigen::MatrixXd::Identity(5,5);
Eigen::MatrixXd Q = Eigen::MatrixXd::Zero(5,5);

void ekfPublisher() {
    nav_msgs::Odometry ekf_msg;

    ekf_msg.header.stamp = ros::Time::now();
    ekf_msg.header.frame_id = "odom";
    ekf_msg.child_frame_id = "base_link";

    ekf_msg.pose.pose.position.x = X(0);
    ekf_msg.pose.pose.position.y = X(1);
    ekf_msg.pose.pose.orientation = tf::createQuaternionMsgFromYaw(X(2));
    ekf_msg.twist.twist.linear.x = X(3);

    for(int i = 0; i < 36; i++) {
        ekf_msg.pose.covariance[i] = 0.0;
        ekf_msg.twist.covariance[i] = 0.0;
    }

    ekf_msg.pose.covariance[0]  = P(0,0);   // x
    ekf_msg.pose.covariance[7]  = P(1,1);   // y
    ekf_msg.pose.covariance[35] = P(2,2);   // yaw

    ekf_msg.pose.covariance[14] = 1e6;  // z
    ekf_msg.pose.covariance[21] = 1e6;  // roll
    ekf_msg.pose.covariance[28] = 1e6;  // pitch

    ekf_msg.twist.covariance[0]  = P(3,3); // vx
    ekf_msg.twist.covariance[35] = P(4,4); // wz

    ekf_msg.twist.covariance[7]  = 1e6; // vy
    ekf_msg.twist.covariance[14] = 1e6; // vz
    ekf_msg.twist.covariance[21] = 1e6; // wx
    ekf_msg.twist.covariance[28] = 1e6; // wy

    ekf_msg.twist.twist.angular.z = X(4);

    ekf_pub.publish(ekf_msg);
}

void imuCallBack(const sensor_msgs::Imu::ConstPtr& msg) {
    imu_msg = *msg;
    last_imu_time = msg->header.stamp;
    imu_received = true;
}

void leftHallCallBack(const boost::shared_ptr<const custom_msgs::EncoderTicks>& msg) {
    double dt = (msg->header.stamp - last_l_hall_time).toSec();
    if (!first_l_hall && dt > 0.0) {
        int32_t delta = msg->ticks - last_l_hall_ticks;
        // Adjusted to 60.0 PPR based on HallEncoderNode config
        l_hall_w = (delta / 60.0) * 2 * M_PI / dt; 
    }
    last_l_hall_ticks = msg->ticks;
    last_l_hall_time = msg->header.stamp;
    first_l_hall = false;
}

void rightHallCallBack(const boost::shared_ptr<const custom_msgs::EncoderTicks>& msg) {
    double dt = (msg->header.stamp - last_r_hall_time).toSec();
    if (!first_r_hall && dt > 0.0) {
        int32_t delta = msg->ticks - last_r_hall_ticks;
        // Adjusted to 60.0 PPR based on HallEncoderNode config
        r_hall_w = (delta / 60.0) * 2 * M_PI / dt; 
    }
    last_r_hall_ticks = msg->ticks;
    last_r_hall_time = msg->header.stamp;
    first_r_hall = false;
}

void steeringCallBack(const std_msgs::Float64::ConstPtr& msg) {
    // Arduino SteeringNode publishes in degrees, convert to radians
    s_enc_angle = msg->data * M_PI / 180.0; 
    
    // Clamp to hardware kinematic limits
    if(s_enc_angle > max_steer) s_enc_angle = max_steer;
    if(s_enc_angle < -max_steer) s_enc_angle = -max_steer;
    
    steering_received = true;
}

void gpsCallBack(const sensor_msgs::NavSatFix::ConstPtr& msg) {
    gps_msg = *msg;
    last_gps_time = msg->header.stamp;

    if(gps_msg.status.status >= sensor_msgs::NavSatStatus::STATUS_FIX) {
        if(!gps_initialized) {
            lat0 = gps_msg.latitude * M_PI / 180.0;
            lng0 = gps_msg.longitude * M_PI / 180.0;
            gps_initialized = true;
        }

        double lat = gps_msg.latitude * M_PI / 180.0;
        double lng = gps_msg.longitude * M_PI / 180.0;

        x_meas = (lng - lng0) * cos(lat0) * R_earth;
        y_meas = (lat - lat0) * R_earth;
    }
}

void ekfPredict(double dt, double v_meas, double w_kin) {
    if(dt <= 0.0) return;

    double x   = X(0);
    double y   = X(1);
    double yaw = X(2);
    double v   = v_meas;
    double w   = w_kin;

    X(0) = x + v * cos(yaw) * dt;
    X(1) = y + v * sin(yaw) * dt;

    X(2) = yaw + w * dt;
    X(2) = atan2(sin(X(2)), cos(X(2)));
    
    X(3) = v;
    X(4) = w;

    Eigen::MatrixXd F = Eigen::MatrixXd::Identity(5,5);

    F(0,2) = -v * sin(yaw) * dt;
    F(0,3) = cos(yaw) * dt;

    F(1,2) = v * cos(yaw) * dt;
    F(1,3) = sin(yaw) * dt;

    F(2,4) = dt;

    P = F * P * F.transpose() + Q * dt;
}

void ekfCorrectIMU(double yaw_meas, double w_meas) {
    Eigen::VectorXd z(2);
    z << yaw_meas, w_meas;

    Eigen::MatrixXd H(2,5);
    H.setZero();
    H(0,2) = 1;
    H(1,4) = 1;

    Eigen::MatrixXd R = Eigen::MatrixXd::Zero(2,2);
    R(0,0) = imu_msg.orientation_covariance[8]; 
    R(1,1) = imu_msg.angular_velocity_covariance[8]; 

    Eigen::VectorXd y = z - H * X;
    y(0) = atan2(sin(y(0)), cos(y(0)));

    Eigen::MatrixXd S = H * P * H.transpose() + R;
    Eigen::MatrixXd K = P * H.transpose() * S.inverse();

    X = X + K * y;
    X(2) = atan2(sin(X(2)), cos(X(2)));

    Eigen::MatrixXd I = Eigen::MatrixXd::Identity(5,5);
    P = (I - K * H) * P * (I - K * H).transpose() + K * R * K.transpose();
}

void ekfCorrectGPS(double x_meas, double y_meas) {
    Eigen::VectorXd z(2);
    z << x_meas, y_meas;

    Eigen::MatrixXd H(2,5);
    H.setZero();
    H(0,0) = 1;
    H(1,1) = 1;

    Eigen::MatrixXd R = Eigen::MatrixXd::Zero(2,2);
    R(0,0) = gps_msg.position_covariance[0];
    R(1,1) = gps_msg.position_covariance[4];

    Eigen::VectorXd y = z - H * X;
    Eigen::MatrixXd S = H * P * H.transpose() + R;
    Eigen::MatrixXd K = P * H.transpose() * S.inverse();

    X = X + K * y;

    Eigen::MatrixXd I = Eigen::MatrixXd::Identity(5,5);
    P = (I - K * H) * P * (I - K * H).transpose() + K * R * K.transpose();
}

int main(int argc, char **argv) {
    ros::init(argc, argv, "ekf_localization_node");
    ros::NodeHandle nh;

    Q(0,0) = 0.05;   // x
    Q(1,1) = 0.05;   // y
    Q(2,2) = 0.02;   // yaw
    Q(3,3) = 0.5;    // v
    Q(4,4) = 0.5;    // w

    ros::Time now = ros::Time::now();
    last_l_hall_time = now;
    last_r_hall_time = now;
    last_imu_time = now;
    last_gps_time = now;

    // Check these topics against your actual ROS graph
    l_hall_sub = nh.subscribe("/hall/left/raw", 10, leftHallCallBack);
    r_hall_sub = nh.subscribe("/hall/right/raw", 10, rightHallCallBack);
    
    // Steering subscribes to processed angle
    s_enc_sub = nh.subscribe("/steering/current_angle", 10, steeringCallBack); 
    
    imu_sub = nh.subscribe("/imu/data", 10, imuCallBack);
    gps_sub = nh.subscribe("/ublox/fix", 10, gpsCallBack);

    ekf_pub = nh.advertise<nav_msgs::Odometry>("/localization/ekf_odom", 10);

    ros::Rate loop_rate(50);

    while (ros::ok()) {
        ros::spinOnce();

        if(!ekf_initialized && gps_initialized && imu_received) {
            tf::Quaternion q;
            tf::quaternionMsgToTF(imu_msg.orientation, q);
            double roll, pitch, yaw_meas;
            tf::Matrix3x3(q).getRPY(roll, pitch, yaw_meas);

            X(0) = x_meas;
            X(1) = y_meas;
            X(2) = yaw_meas;
            X(3) = 0.0;
            X(4) = 0.0;

            ekf_initialized = true;
            ROS_WARN("EKF initialized to GPS + IMU origin");
        }

        static ros::Time last_time = ros::Time::now();
        ros::Time current_time = ros::Time::now();
        double dt = (current_time - last_time).toSec();
        last_time = current_time;

        if(dt <= 0.0 || dt > 0.1) {
            ROS_WARN("Large dt detected: %f", dt);
            dt = 0.02;
        }

        double v_meas = wheel_radius * (l_hall_w + r_hall_w) / 2.0;

        // Kinematic w calculation uses processed s_enc_angle
        double w_kin = 0.0;
        if (steering_received) {
            w_kin = v_meas / wheelbase * tan(s_enc_angle);
        }

        if(ekf_initialized) {
            ekfPredict(dt, v_meas, w_kin);
        }

        if(imu_received && last_imu_time > last_used_imu_time) {
            tf::Quaternion q;
            tf::quaternionMsgToTF(imu_msg.orientation, q);
            double roll, pitch, yaw_meas;
            tf::Matrix3x3(q).getRPY(roll, pitch, yaw_meas);
            double w_meas = imu_msg.angular_velocity.z;

            ekfCorrectIMU(yaw_meas, w_meas);
            last_used_imu_time = last_imu_time;
        }
        
        if(gps_initialized && last_gps_time > last_used_gps_time) {
            ekfCorrectGPS(x_meas, y_meas);
            last_used_gps_time = last_gps_time;
        }
        
        ekfPublisher();
        loop_rate.sleep();
    }

    return 0;
}