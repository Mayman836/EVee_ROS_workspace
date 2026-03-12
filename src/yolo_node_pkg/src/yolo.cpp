#include <ros/ros.h>
#include <sensor_msgs/Image.h>
#include <vision_msgs/Detection2DArray.h>
#include <vision_msgs/Detection2D.h>
#include <vision_msgs/ObjectHypothesisWithPose.h>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>

#include <NvInfer.h>
#include <cuda_runtime_api.h>

#include <vector>
#include <string>
#include <fstream>
#include <algorithm>

struct LetterboxInfo{
    float scale;
    int pad_x;
    int pad_y;
};

struct TrackedDet{
    cv::Rect box;
    int class_id;
    float score;
    int age;
    int missed;
};

struct Det{
    cv::Rect box;
    int class_id;
    float score;
};

static std::vector<TrackedDet> tracked;

static const float IOU_THRESH = 0.3f;
static const int MAX_MISSES = 5;

class Logger : public nvinfer1::ILogger{
public:
    void log(Severity severity,const char* msg) noexcept override{
        if(severity<=Severity::kWARNING) std::cout<<msg<<std::endl;
    }
} gLogger;

nvinfer1::IRuntime* runtime=nullptr;
nvinfer1::ICudaEngine* engine=nullptr;
nvinfer1::IExecutionContext* context=nullptr;

void* buffers[2];
cudaStream_t stream;

int inputIndex;
int outputIndex;

ros::Subscriber img_sub;
ros::Publisher yolo_pub;

static float input_tensor[3*640*640];
static float output_tensor[7*8400];

static std::vector<cv::Rect> boxes;
static std::vector<float> scores;
static std::vector<int> class_ids;
static std::vector<int> keep;
static std::vector<Det> detections;

LetterboxInfo letterbox(const cv::Mat& src,cv::Mat& dst,int target=640)
{
    int w=src.cols;
    int h=src.rows;

    float scale=std::min(target/(float)w,target/(float)h);

    int new_w=int(w*scale);
    int new_h=int(h*scale);

    cv::resize(src,dst,cv::Size(new_w,new_h));

    int pad_x=(target-new_w)/2;
    int pad_y=(target-new_h)/2;

    cv::copyMakeBorder(
        dst,dst,
        pad_y,target-new_h-pad_y,
        pad_x,target-new_w-pad_x,
        cv::BORDER_CONSTANT,
        cv::Scalar(114,114,114)
    );

    return{scale,pad_x,pad_y};
}

float IoU(const cv::Rect&a,const cv::Rect&b)
{
    int inter=(a&b).area();
    int uni=a.area()+b.area()-inter;
    return uni>0?(float)inter/uni:0.f;
}

bool loadEngine(const std::string& path)
{
    std::ifstream file(path,std::ios::binary);

    if(!file.good()){
        ROS_ERROR("Engine file not found");
        return false;
    }

    file.seekg(0,file.end);
    size_t size=file.tellg();
    file.seekg(0,file.beg);

    std::vector<char> buffer(size);
    file.read(buffer.data(),size);
    file.close();

    runtime=nvinfer1::createInferRuntime(gLogger);
    engine=runtime->deserializeCudaEngine(buffer.data(),size);

    if(!engine){
        ROS_ERROR("TensorRT failed to deserialize engine");
        return false;
    }

    context=engine->createExecutionContext();

    inputIndex=engine->getBindingIndex("images");
    outputIndex=engine->getBindingIndex("output0");

    cudaMalloc(&buffers[inputIndex],3*640*640*sizeof(float));
    cudaMalloc(&buffers[outputIndex],7*8400*sizeof(float));

    cudaStreamCreate(&stream);

    ROS_INFO("TensorRT engine ready");

    return true;
}

void yoloCallback(const sensor_msgs::ImageConstPtr& img_msg)
{
    ROS_INFO("Image received");

    cv::Mat frame = cv_bridge::toCvCopy(img_msg, "bgr8")->image;

    if(frame.empty()){
        ROS_WARN("Empty frame received");
        return;
    }

    cv::Mat resized;
    LetterboxInfo lb = letterbox(frame,resized,640);

    cv::cvtColor(resized,resized,cv::COLOR_BGR2RGB);

    cv::Mat blob;
    cv::dnn::blobFromImage(
        resized,
        blob,
        1.0/255.0,
        cv::Size(640,640),
        cv::Scalar(),
        true,
        false
    );

    memcpy(input_tensor, blob.ptr<float>(), 3*640*640*sizeof(float));

    cudaMemcpyAsync(buffers[inputIndex],input_tensor,sizeof(input_tensor),cudaMemcpyHostToDevice,stream);

    if(!context->enqueueV2(buffers,stream,nullptr)){
        ROS_ERROR("TensorRT enqueue failed");
        return;
    }

    cudaMemcpyAsync(output_tensor,buffers[outputIndex],sizeof(output_tensor),cudaMemcpyDeviceToHost,stream);

    cudaStreamSynchronize(stream);

    ROS_INFO("Inference finished");

    boxes.clear();
    scores.clear();
    class_ids.clear();
    detections.clear();
    keep.clear();

    const int num_preds=8400;
    const int num_vals=7;

    float conf_thresh=0.25f;
    float nms_thresh=0.45f;

    float* out=output_tensor;

    for(int i=0;i<num_preds;i++)
    {
        float cx=out[0*num_preds+i];
        float cy=out[1*num_preds+i];
        float w=out[2*num_preds+i];
        float h=out[3*num_preds+i];

        float best_score=0.f;
        int best_class=-1;

        for(int c=0;c<num_vals-4;c++)
        {
            float s=out[(4+c)*num_preds+i];

            if(s>best_score){
                best_score=s;
                best_class=c;
            }
        }

        if(best_score<conf_thresh) continue;

        float x1=(cx-w/2-lb.pad_x)/lb.scale;
        float y1=(cy-h/2-lb.pad_y)/lb.scale;
        float x2=(cx+w/2-lb.pad_x)/lb.scale;
        float y2=(cy+h/2-lb.pad_y)/lb.scale;

        x1=std::max(0.f,std::min(x1,(float)frame.cols));
        y1=std::max(0.f,std::min(y1,(float)frame.rows));
        x2=std::max(0.f,std::min(x2,(float)frame.cols));
        y2=std::max(0.f,std::min(y2,(float)frame.rows));

        boxes.emplace_back(cv::Rect(cv::Point((int)x1,(int)y1),cv::Point((int)x2,(int)y2)));

        scores.push_back(best_score);
        class_ids.push_back(best_class);
    }

    ROS_INFO("Boxes before NMS: %lu", boxes.size());

    cv::dnn::NMSBoxes(boxes,scores,conf_thresh,nms_thresh,keep);

    for(int k:keep)
        detections.push_back({boxes[k],class_ids[k],scores[k]});

    std::vector<bool> used(detections.size(),false);

    for(auto& t:tracked)
    {
        float best_iou=0;
        int best=-1;

        for(size_t i=0;i<detections.size();i++)
        {
            if(used[i]) continue;
            if(detections[i].class_id!=t.class_id) continue;

            float iou=IoU(t.box,detections[i].box);

            if(iou>best_iou){
                best_iou=iou;
                best=i;
            }
        }

        if(best_iou>IOU_THRESH)
        {
            t.box=detections[best].box;
            t.score=detections[best].score;
            t.age++;
            t.missed=0;
            used[best]=true;
        }
        else
            t.missed++;
    }

    for(size_t i=0;i<detections.size();i++)
        if(!used[i])
            tracked.push_back({detections[i].box,detections[i].class_id,detections[i].score,1,0});

    tracked.erase(
        std::remove_if(tracked.begin(),tracked.end(),
            [](const TrackedDet&t){return t.missed>MAX_MISSES;}
        ),
        tracked.end()
    );

    ROS_INFO("Tracked objects: %lu", tracked.size());

    vision_msgs::Detection2DArray yolo_array;
    yolo_array.header = img_msg->header;

    yolo_array.detections.resize(tracked.size());

    for(size_t i=0;i<tracked.size();i++)
    {
        const auto& t = tracked[i];
        auto& det = yolo_array.detections[i];

        det.header = yolo_array.header;

        det.bbox.center.x = t.box.x + t.box.width * 0.5;
        det.bbox.center.y = t.box.y + t.box.height * 0.5;
        det.bbox.size_x = t.box.width;
        det.bbox.size_y = t.box.height;

        det.results.resize(1);
        det.results[0].id = t.class_id;
        det.results[0].score = t.score;
    }

    cv::Mat display = frame.clone();

    for(const auto& t : tracked)
    {
        cv::rectangle(display,t.box,cv::Scalar(0,255,0),2);

        std::string label =
            std::to_string(t.class_id) + " " +
            std::to_string(t.score).substr(0,4);

        cv::putText(display,label,cv::Point(t.box.x,t.box.y-5),
                    cv::FONT_HERSHEY_SIMPLEX,0.5,cv::Scalar(0,255,0),2);
    }

    cv::imshow("YOLO Detection", display);
    cv::waitKey(1);

    ROS_INFO("Publishing detections");

    yolo_pub.publish(yolo_array);
}

int main(int argc,char** argv)
{
    ros::init(argc,argv,"yolo_node");
    ros::NodeHandle nh;

    ROS_INFO("Starting YOLO node");

    cudaSetDevice(0);

    ROS_INFO("Loading TensorRT engine...");

    if(!loadEngine("/home/evee/ROS/models/best.engine")){
        ROS_ERROR("Engine FAILED to load");
        return -1;
    }

    ROS_INFO("Engine loaded successfully");

    cv::namedWindow("YOLO Detection", cv::WINDOW_NORMAL);

    yolo_pub=nh.advertise<vision_msgs::Detection2DArray>("/vision/detections",10);

    img_sub=nh.subscribe("/camera/image_raw",2,yoloCallback);

    ROS_INFO("YOLO node started and waiting for images");

    ros::spin();
    return 0;
}