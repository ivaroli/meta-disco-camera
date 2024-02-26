#include <iostream>
#include <vector>
#include <VmbCPP/VmbCPP.h>
#include <opencv2/opencv.hpp>
#include "vimba_provider.hpp"
#include "exposure_helper.hpp"
#include "types.hpp"
#include <filesystem>
#include <ctime>
#include <bits/stdc++.h>
#include <chrono>
#include "message_queue.hpp"
#include "csp_server.hpp"

namespace fs = std::filesystem;

using namespace std::chrono;

std::string_view get_option(
    const std::vector<std::string_view>& args, 
    const std::string_view& option_name) {
    for (auto it = args.begin(), end = args.end(); it != end; ++it) {
        if (*it == option_name)
            if (it + 1 != end)
                return *(it + 1);
    }
    
    return "";
}

bool has_option(
    const std::vector<std::string_view>& args, 
    const std::string_view& option_name) {
    for (auto it = args.begin(), end = args.end(); it != end; ++it) {
        if (*it == option_name)
            return true;
    }
    
    return false;
}

std::vector<std::string> split_string(std::string input){
    std::istringstream iss(input);
    std::string word;
    std::vector<std::string> words;

    while (std::getline(iss, word, ',')) {
        words.push_back(word);
    }

    return words;
}

void print_usage(){
    std::string help = R"""(Usage: Disco2CameraControl -e EXPOSURE [-s] [-n NUM_IMAGES] [-f FEATURES]

Description:
  Perform image processing with the following options:

Required Argument:
  -e EXPOSURE   Set the exposure level (a positive number) or auto.
  -c CAMERA      Specify the camera to be used in the current burst.

Optional Arguments:
  -n NUM_IMAGES  Specify the number of images to save (only valid with -s), default is 1.
    )""";

    std::cout << help << std::endl;
}

void capture(CaptureMessage params, VimbaProvider* vmbProvider, MessageQueue* mq, std::vector<VmbCPP::CameraPtr> cameras){
    VmbCPP::CameraPtr cam;
    
    if(cameras.size() > 0){
        for(int i = 0; i < cameras.size(); i++){
            std::string camName = "";
            cameras.at(i)->GetModel(camName);

            if(camName == params.Camera){
                cam = cameras.at(i);
            }
        }
    } else {
        std::cerr << "No cameras were detected" << std::endl;
        return;
    }

    std::cout << "Size of pointer: " << sizeof(int*) << " bytes" << std::endl;

    if(cam != NULL && params.NumberOfImages > 0){
        cam->Open(VmbAccessModeExclusive);
        float exposure = (params.Exposure == 0)?set_exposure(cam, vmbProvider):params.Exposure;
        VmbCPP::FramePtrVector frames = vmbProvider->AqcuireFrame(cameras.at(0), exposure, 0, params.NumberOfImages);
        
        unsigned int width, height, bufferSize;
        frames.at(0)->GetBufferSize(bufferSize);
        frames.at(0)->GetWidth(width);
        frames.at(0)->GetHeight(height);

        unsigned char* total_buffer = new unsigned char[bufferSize*params.NumberOfImages];

        for(int i = 0; i < params.NumberOfImages; i++){
            unsigned char* buffer;
            frames.at(i)->GetImage(buffer);
            std::memcpy((void*)(&total_buffer[i * bufferSize]), buffer, bufferSize * sizeof(unsigned char));
        }

        ImageBatch batch;
        batch.height = height;
        batch.width = width;
        batch.channels = 1;
        batch.num_images = params.NumberOfImages;
        batch.data_size = bufferSize*params.NumberOfImages;
        batch.data = total_buffer;

        if(mq->SendImage(batch)){
            std::cout << "Sending image was successful" << std::endl;
        } else {
            std::cout << "Sending image was unsuccessful" << std::endl;
        }
        
        delete[] total_buffer;
        cam->Close();
    } else {
        if(params.NumberOfImages <= 0){
            std::cerr << "Number of images must be greater than zero" << std::endl;
        }

        if(cam == NULL){
            std::cerr << "Camera must be one of: ";

            for(int i = 0; i < cameras.size(); i++){
                std::string camName;
                cameras.at(i)->GetModel(camName);
                std::cerr << camName << " ";
            }

            std::cerr << std::endl;
        }
    }
}

int main(int argc, char *argv[], char *envp[]){
    // parse arguments
    const std::vector<std::string_view> args(argv, argv + argc);
    const std::string_view exposure_arg = get_option(args, "-e");
    const std::string_view camera = get_option(args, "-c");
    const std::string_view num_images_arg = get_option(args, "-n");

    VimbaProvider* vmbProvider = new VimbaProvider();
    MessageQueue* mq = new MessageQueue();
    std::vector<VmbCPP::CameraPtr> cameras = vmbProvider->GetCameras();

    server_init(capture, vmbProvider, mq, cameras);

    delete vmbProvider; 
    delete mq;
    return 0;
}