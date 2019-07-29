/******************************************************************************
*
* The MIT License (MIT)
*
* Copyright (c) 2018 Bluewhale Robot
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Author: Randoms
*******************************************************************************/

#include "qrcode_wifi_node.h"

ros::Publisher audioPub;
ros::Publisher imagePub;
bool isEnabled = true;
int messageCount = 0;
bool newFrameFlag = false;
cv::Mat currentFrame;
std::mutex frameLock;

sensor_msgs::ImageConstPtr lastFrame;

void updateFrame(const sensor_msgs::ImageConstPtr &newFrame)
{
    lastFrame = newFrame;
    if (!isEnabled)
        return;
    messageCount++;
    if (messageCount < 30)
        return;
    messageCount = 0;
    cv_bridge::CvImagePtr cvPtr = cv_bridge::toCvCopy(newFrame, "bgr8");
    cv::Mat cvImage = cvPtr->image;
    {
        std::unique_lock<std::mutex> lock(frameLock);
        currentFrame = cvPtr->image;
        newFrameFlag = true;
    }
}

void ProcessQRCode()
{
    while (ros::ok())
    {
        cv::Mat qrcodeImage;
        {
            if (!newFrameFlag)
            {
                Sleep(500);
                continue;
            }
            std::unique_lock<std::mutex> lock(frameLock);
            qrcodeImage = currentFrame;
            newFrameFlag = false;
        }
        if(exec("iwconfig 2>&1 | grep \"Link Quality\"").size() != 0){
            Sleep(1000);
            continue;
        }
        std::vector<decodedObject> qrcodes;
        decode(qrcodeImage, qrcodes);
        if (qrcodes.size() == 0)
        {
            Sleep(1000);
            continue;
        }
        for (auto it = qrcodes.begin(); it < qrcodes.end(); it++)
        {
            if (it->type != "QR-Code")
                continue;
            auto dataJson = nlohmann::json::parse(it->data, NULL, false);
            if (dataJson == nlohmann::json::value_t::discarded)
                continue;
            if (!dataJson.is_array())
                continue;
            if (dataJson.size() != 3)
                continue;
            auto ssid = dataJson[0];
            auto bssid = dataJson[1];
            auto password = dataJson[2];
            if (!ssid.is_string() || !bssid.is_string() || !password.is_string())
                continue;
            std_msgs::String qrNotify;
            qrNotify.data = "检测到wifi二维码，正在尝试连接";
            audioPub.publish(qrNotify);
            std::stringstream ss;
            ss << "nmcli device wifi connect " << ssid << " password " << password;
            std::string res = exec(ss.str().c_str());
            if (res.find("successfully activated") != std::string::npos)
            {
                qrNotify.data = "连接wifi成功";
                audioPub.publish(qrNotify);
                ROS_INFO_STREAM("连接wifi成功");
                auto ipList = ListIpAddresses();
                if (ipList.size() != 0)
                {
                    qrNotify.data = "当前机器人ip为 " + ipList[0] + "\n";
                    audioPub.publish(qrNotify);
                    ROS_INFO_STREAM("当前机器人ip为 " << ipList[0] << std::endl);
                    Sleep(5000);
                }
            }
            else
            {
                qrNotify.data = "连接wifi失败";
                ROS_INFO_STREAM("连接wifi失败");
                audioPub.publish(qrNotify);
            }
            Sleep(5000);
        }
        Sleep(5000);
    }
}

std::string exec(const char *cmd)
{
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe)
    {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }
    return result;
}

static std::vector<std::string> ListIpAddresses()
{
    struct ifaddrs *ifAddrStruct = NULL;
    struct ifaddrs *ifa = NULL;
    void *tmpAddrPtr = NULL;
    std::vector<std::string> ips;

    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (!ifa->ifa_addr)
        {
            continue;
        }
        if (ifa->ifa_addr->sa_family == AF_INET)
        { // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            std::string ifname(ifa->ifa_name);
            if (ifname.find("lo") == 0 || ifname.find("docker") == 0 || ifname.find("virtual") == 0)
                continue;
            std::string ip = std::string(addressBuffer);
            if (ip.find("0.") == 0)
            {
                continue;
            }
            ips.push_back(addressBuffer);
        }
    }
    if (ifAddrStruct != NULL)
        freeifaddrs(ifAddrStruct);
    return ips;
}

void Sleep(int time)
{
    usleep(time * 1000);
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "xiaoqiang_greeting_node");
    ros::AsyncSpinner spinner(4);
    spinner.start();
    ros::NodeHandle private_nh("~");
    audioPub = private_nh.advertise<std_msgs::String>("text", 10);
    ros::Subscriber imageSub = private_nh.subscribe("image", 1, updateFrame);
    imagePub = private_nh.advertise<sensor_msgs::Image>("processed_image", 10);
    // ros::param::param<std::string>("~greeting_words", greeting_words, "欢迎光临");
    std::thread t(ProcessQRCode);
    while (ros::ok())
    {
        private_nh.param("is_enabled", isEnabled, true);
        sleep(1);
    }
}