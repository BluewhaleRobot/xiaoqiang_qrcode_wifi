#include <opencv2/opencv.hpp>
#include <zbar.h>
#include <ros/ros.h>

typedef struct
{
  std::string type;
  std::string data;
  std::vector <cv::Point> location;
} decodedObject;

void decode(cv::Mat &im, std::vector<decodedObject>&decodedObjects);