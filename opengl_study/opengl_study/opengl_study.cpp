


#include <vector>
#include <cstdio>
#include <cmath>
#include <future>
#include <conio.h>

#include <opencv2/opencv.hpp>

#include "render.h"
#include "render_runtime.h"



void callback(const hv::v1::frameView& frame) {

    cv::Mat cvImage(cv::Size(frame.width, frame.height), CV_8UC4, frame.data, frame.stride);
    cv::Mat cvtColor;


    cv::cvtColor(cvImage, cvtColor, cv::COLOR_RGBA2BGRA);


    cv::namedWindow("render result");
    cv::imshow("render result", cvtColor);
    cv::waitKey(1);

}

void callback2(const hv::v1::frameView& frame) {

    cv::Mat cvImage(cv::Size(frame.width, frame.height), CV_8UC4, frame.data, frame.stride);
    cv::Mat cvtColor;


    cv::cvtColor(cvImage, cvtColor, cv::COLOR_RGBA2BGRA);


    cv::namedWindow("render result2");
    cv::imshow("render result2", cvtColor);
    cv::waitKey(1);

}


int main() {
    

    hv::v1::render render;
    render.registerFrameCallback(callback);
    render.resizeOutput({512,512});
    

    hv::v1::render render2;
    render2.registerFrameCallback(callback2);
    render2.resizeOutput({ 512,512 });


    while (true) {
        char input_key = _getch();

        switch (input_key)
        {
        case '=':
            std::cout << "zoom out" << std::endl;
            render.zoomBy(1.1f);
            render2.zoomBy(1.1f);
            break;
        case '-':
            std::cout << "zoom in" << std::endl;
            render.zoomBy(0.9f);
            render2.zoomBy(0.9f);
            break;
        case 72:
            std::cout << "y +" << std::endl;
            render.translateBy(0, -1);
            render2.translateBy(0, 1);
            break;
        case 80:
            std::cout << "y -" << std::endl;
            render.translateBy(0, 1);
            render2.translateBy(0, -1);
            break;
        case 75:
            std::cout << "x -" << std::endl;
            render.translateBy(-1, 0);
            render2.translateBy(1, 0);
            break;
        case 77:
            std::cout << "x +" << std::endl;
            render.translateBy(1, 0);
            render2.translateBy(-1, 0);
            break;

        case 's':
            std::cout << "pitch - " << std::endl;
            render.orbitBy(0, -3);
            render2.orbitBy(0, 3);
            break;

        case 'w':
            std::cout << "pitch + " << std::endl;
            render.orbitBy(0, 3);
            render2.orbitBy(0, -3);
            break;

        case 'd':
            std::cout << "yaw + " << std::endl;
            render.orbitBy(3, 0);
            render2.orbitBy(-3, 0);
            break;

        case 'a':
            std::cout << "yaw - " << std::endl;
            render.orbitBy(-3, 0);
            render2.orbitBy(3, 0);
            break;

        case 'c':
            std::cout << "exit" << std::endl;
            return 0;
        default:
            break;
        }
    }


    return 0;
}