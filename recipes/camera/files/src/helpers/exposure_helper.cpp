#include <iostream>
#include <opencv2/opencv.hpp>
#include <cmath>
#include <VmbCPP/VmbCPP.h>
#include "exposure_helper.hpp"
#include "vimba_provider.hpp"

using namespace std;
using namespace cv;

// auto exposure paramteres
const float MAX_EXPOSURE = 1000000.0; // maximum allowed exposure
const float MAX_ENTROPY = 400.923; // maximum entropy achieved by a image with 3 channels
const size_t STEPS = 20;
const float MIN_EXPOSURE = 10000.0;
const float LEARNING_RATE = 20000;

double calculateEntropy(Mat image) {
    if (image.empty()) {
        cerr << "Error: Input image is empty." << endl;
        return 0.0;
    }

    if (image.channels() > 3) {
        cvtColor(image, image, COLOR_BGR2RGB);
    }

    int hist_r[256]={0}, hist_g[256]={0}, hist_b[256]={0};
    int totalPixels = image.rows * image.cols;

    for (int i = 0; i < image.rows; i++) {
        for (int j = 0; j < image.cols; j++) {
            cv::Vec3b pixel = image.at<cv::Vec3b>(i,j);
            int red = static_cast<int>(pixel[0]), green = static_cast<int>(pixel[1]), blue = static_cast<int>(pixel[2]);

            hist_r[red]++;
            hist_g[green]++;
            hist_b[blue]++;
        }
    }

    double total_r = 0.0, total_g = 0.0, total_b = 0.0;
    for (int i = 0; i < 256; i++) {
        if (hist_r[i] > 0) {
            double probability = static_cast<double>(hist_r[i]) / totalPixels;
            total_r += probability * log2(probability);
        }

        if (hist_g[i] > 0) {
            double probability = static_cast<double>(hist_g[i]) / totalPixels;
            total_g += probability * log2(probability);
        }

        if (hist_b[i] > 0) {
            double probability = static_cast<double>(hist_b[i]) / totalPixels;
            total_b += probability * log2(probability);
        }
    }

    return -total_r-total_g-total_b;
}

float set_exposure(VmbCPP::CameraPtr cam, VimbaProvider* p){
    float currentExposure = MIN_EXPOSURE, lastEntropy = -1, lastExposure = -1, slope = 1;
    size_t steps = 0;

    while(currentExposure < MAX_EXPOSURE && steps < STEPS){
        VmbCPP::FramePtrVector frames = p->AqcuireFrame(cam, currentExposure, 0, 1);            
        VmbCPP::FramePtr frame = frames.at(0);
        unsigned int size, width, height;
        frame->GetBufferSize(size);
        frame->GetHeight(height);
        frame->GetWidth(width);
        
        unsigned char* buffer;
        frame->GetImage(buffer);

        cv::Mat img(height, width, CV_8UC3, buffer);
        cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
        float currentEntropy = calculateEntropy(img);

        if(lastEntropy == -1){
            lastEntropy = currentEntropy;
            lastExposure = currentExposure;

            currentExposure += LEARNING_RATE;
        } else {
            float exposure_delta = currentExposure - lastExposure;
            float entropy_delta = currentEntropy - lastEntropy;
            slope = (entropy_delta/MAX_ENTROPY)/(exposure_delta/MAX_EXPOSURE); // normalized slope

            lastEntropy = currentEntropy;
            lastExposure = currentExposure;

            currentExposure += LEARNING_RATE*slope;
            std::cout << slope << " " << currentExposure << std::endl;
        }

        steps++;
    }

    return currentExposure;
}