# xiaoqiang_qrcode_wifi

## 功能说明

小强通过二维码连接wifi功能。通过在手机小程序上输入WiFi信息生成二维码。机器人通过摄像头扫描二维码获取WiFi信息,自动连接对应WiFi。

## 使用

```bash
roslaunch xiaoqiang_qrcode_wifi xiaoqiang_qrcode_wifi.launch
```

### 订阅话题

|话题|类型|说明|
|--|--|--|
|image|sensor_msgs/Image|摄像头图像输入话题|

### 参数说明

请参照 [xiaoqiang_qrcode_wifi.launch](./launch/xiaoqiang_qrcode_wifi.launch) 文件注释
