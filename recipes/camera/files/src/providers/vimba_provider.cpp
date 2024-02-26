#include <iostream>
#include <stdexcept>
#include <VmbCPP/VmbCPP.h>
#include "vimba_provider.hpp"

using namespace VmbCPP;

VimbaProvider::VimbaProvider() : sys(VmbSystem::GetInstance()){
    VmbErrorType err = this->sys.Startup();

    if (err != VmbErrorSuccess)
    {
        /* code */
    }

    if (VmbErrorSuccess != err)
    {
        throw std::runtime_error("Could not start system. Error code: " + std::to_string(err) );
    }
}

VimbaProvider::~VimbaProvider(){
    sys.Shutdown();

    // for(CameraPtr cam : cameras){
    //     cam->Close();
    // }
}

void GigEAdjustPacketSize(CameraPtr camera)
{
    StreamPtrVector streams;
    VmbErrorType err = camera->GetStreams(streams);

    if (err != VmbErrorSuccess || streams.empty())
    {
        throw std::runtime_error("Could not get stream modules, err=" + std::to_string(err));
    }

    FeaturePtr feature;
    err = streams[0]->GetFeatureByName("GVSPAdjustPacketSize", feature);

    if (err == VmbErrorSuccess)
    {
        err = feature->RunCommand();
        if (err == VmbErrorSuccess)
        {
            bool commandDone = false;
            do
            {
                if (feature->IsCommandDone(commandDone) != VmbErrorSuccess)
                {
                    break;
                }
            } while (commandDone == false);
        }
        else
        {
            std::cout << "Error while executing GVSPAdjustPacketSize, err=" + std::to_string(err) << std::endl;
        }
    }
}

double VimbaProvider::GetFeature(std::string feature_name, VmbCPP::CameraPtr camera){
    double feature_value;
    FeaturePtr feature;
    VmbErrorType err = camera->GetFeatureByName(feature_name.c_str(), feature);

    if (err == VmbErrorSuccess)
    {
        err = feature->GetValue(feature_value);
        
        return feature_value;
    } else {
        std::cout << err << std::endl;
    }

    return 0;
}

std::vector<CameraPtr> VimbaProvider::GetCameras(){
    CameraPtrVector cams;
    VmbErrorType err = sys.GetCameras(cams);

    if (VmbErrorSuccess == err)
    {
        return cams;
    }
    else
    {
        sys.Shutdown();
        throw std::runtime_error("Could not list cameras. Error code: " + std::to_string(err) );
    }
}

FramePtrVector VimbaProvider::AqcuireFrame(VmbCPP::CameraPtr cam, float exposure, float gain, int numFrames){
    FeaturePtr pFormatFeature;

    // Set pixel format. For the sake of simplicity we only support Mono and BGR in this example.
    VmbErrorType err = cam->GetFeatureByName( "PixelFormat", pFormatFeature );
    
    if ( VmbErrorSuccess == err )
    {
        // Try to set BGR
        err = pFormatFeature->SetValue( VmbPixelFormatBayerGR8 );

        if ( VmbErrorSuccess != err ){
            std::cout << "ERROR CHANING PIXEL FORMAT" << std::endl;
        }

        // Try to set exposure time: GigE: ExposureTimeAbs; Alvium USB3: ExposureTime
        FeaturePtr pExposureFeature, pGainFeature;
        
        err = cam->GetFeatureByName("ExposureTimeAbs", pExposureFeature);
        if ( VmbErrorSuccess != err ) { // try USB3 feature name
            err = cam->GetFeatureByName("ExposureTime", pExposureFeature);
        }

        if (VmbErrorSuccess == err) {
            err = pExposureFeature->SetValue(exposure); // in us, 15000.0us = 15ms
        }

        err = cam->GetFeatureByName("Gain", pGainFeature);

        if (VmbErrorSuccess == err) {
            err = pGainFeature->SetValue(gain); // in us, 15000.0us = 15ms
        }
    }

    FramePtrVector frames;

    for(int i = 0; i < numFrames; i++){
        FramePtr frame;
        frames.push_back(frame);
    }

    err = cam->AcquireMultipleImages(frames, 5000);

    if (err != VmbErrorSuccess)
    {
        throw std::runtime_error("Could not acquire frame, err=" + std::to_string(err));
    }

    return frames;
}