#include <opencv2/opencv.hpp>
#include <cmath>
#include "vimba_provider.hpp"

#ifndef EXPOSUREHELPER_H
#define EXPOSUREHELPER_H

double calculateEntropy(cv::Mat image);

float set_exposure(VmbCPP::CameraPtr cam, VimbaProvider* p);

#endif