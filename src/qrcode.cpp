#include "qrcode.hpp"

// Find and decode barcodes and QR codes
void decode(cv::Mat &im, std::vector<decodedObject> &decodedObjects)
{
    // Create zbar scanner
    zbar::ImageScanner scanner;

    // Configure scanner
    scanner.set_config(zbar::ZBAR_NONE, zbar::ZBAR_CFG_ENABLE, 1);

    // Convert image to grayscale
    cv::Mat imGray, edges;
    cvtColor(im, imGray, CV_BGR2GRAY);
    // if (!detect_cv(imGray))
    //     return;
    // Wrap image data in a zbar image
    zbar::Image image(im.cols, im.rows, "Y800", (uchar *)imGray.data, im.cols * im.rows);

    ROS_INFO_STREAM("Start scan");
    // Scan the image for barcodes and QRCodes
    int n = scanner.scan(image);
    ROS_INFO_STREAM("n: " << n);
    // Print results
    for (zbar::Image::SymbolIterator symbol = image.symbol_begin(); symbol != image.symbol_end(); ++symbol)
    {
        decodedObject obj;

        obj.type = symbol->get_type_name();
        obj.data = symbol->get_data();

        // Print type and data
        ROS_INFO_STREAM("Type : " << obj.type);
        ROS_INFO_STREAM("Data : " << obj.data);

        // Obtain location
        for (int i = 0; i < symbol->get_location_size(); i++)
        {
            obj.location.push_back(cv::Point(symbol->get_location_x(i), symbol->get_location_y(i)));
        }

        decodedObjects.push_back(obj);
    }
}