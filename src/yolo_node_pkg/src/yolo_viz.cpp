#include <ros/ros.h>
#include <sensor_msgs/Image.h>
#include <vision_msgs/Detection2DArray.h>
#include <mutex>

class ImmortalVisualizer {
private:
    ros::NodeHandle nh;
    ros::Subscriber sub_img, sub_det;
    ros::Publisher pub_viz;
    vision_msgs::Detection2DArray last_det;
    std::mutex mtx;
    bool has_det = false;

    // Helper to draw a single pixel safely
    void drawPixel(sensor_msgs::Image& img, int x, int y) {
        if (x >= 0 && x < (int)img.width && y >= 0 && y < (int)img.height) {
            size_t idx = (y * img.step) + (x * 3);
            img.data[idx] = 0;     // Blue
            img.data[idx + 1] = 255; // Green
            img.data[idx + 2] = 0;   // Red
        }
    }

public:
    ImmortalVisualizer() {
        sub_img = nh.subscribe("/camera/cam1/image_raw", 1, &ImmortalVisualizer::imgCb, this);
        sub_det = nh.subscribe("/yolo/detections", 1, &ImmortalVisualizer::detCb, this);
        pub_viz = nh.advertise<sensor_msgs::Image>("/yolo/viz", 1);
    }

    void detCb(const vision_msgs::Detection2DArray::ConstPtr& msg) {
        std::lock_guard<std::mutex> lock(mtx);
        last_det = *msg;
        has_det = true;
    }

    void imgCb(const sensor_msgs::ImageConstPtr& msg) {
        // Create a deep copy of the message container and data
        sensor_msgs::Image out = *msg; 

        std::lock_guard<std::mutex> lock(mtx);
        if (has_det) {
            for (const auto& det : last_det.detections) {
                int w = (int)det.bbox.size_x;
                int h = (int)det.bbox.size_y;
                int x1 = (int)(det.bbox.center.x - w/2.0);
                int y1 = (int)(det.bbox.center.y - h/2.0);
                int x2 = x1 + w;
                int y2 = y1 + h;

                // Draw Top and Bottom lines
                for (int x = x1; x <= x2; ++x) {
                    drawPixel(out, x, y1);
                    drawPixel(out, x, y2);
                    // Double thickness for visibility
                    drawPixel(out, x, y1 + 1);
                    drawPixel(out, x, y2 - 1);
                }
                // Draw Left and Right lines
                for (int y = y1; y <= y2; ++y) {
                    drawPixel(out, x1, y);
                    drawPixel(out, x2, y);
                    // Double thickness
                    drawPixel(out, x1 + 1, y);
                    drawPixel(out, x2 - 1, y);
                }
            }
        }
        pub_viz.publish(out);
    }
};

int main(int argc, char** argv) {
    ros::init(argc, argv, "yolo_visualizer_node");
    ImmortalVisualizer iv;
    ros::spin();
    return 0;
}