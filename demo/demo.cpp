#include "imageview/imageview.h"
#include <iostream>
#include "opencv2/opencv.hpp"



int main() {

    cv::Mat img = cv::imread("/home/ych/cpp/test_gtk/bs.jpg");
    imageview_show("img",img);


    return 0;
}
