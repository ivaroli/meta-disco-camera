/*=============================================================================
  Copyright (C) 2012 - 2022 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:         AcquisitionHelperChunk.cpp

  Description:  Helper class for acquiring images with VmbCPP
                and accessing Chunk data

-------------------------------------------------------------------------------

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF TITLE,
  NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR  PURPOSE ARE
  DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=============================================================================*/

#include "AcquisitionHelperChunk.h"

#include <VmbCPP/VmbCPP.h>

#include <exception>
#include <iostream>


namespace VmbCPP {
namespace Examples {

/**
 * \brief Helper function to adjust the packet size for Allied vision GigE cameras
 */
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

/**
 * \brief Helper function to retrieve a feature value as std::string
 * 
 * \param[in]  feat Feature to query the value from
 * \param[out] val  Value as string
 */
VmbErrorType GetFeatureValueAsString(FeaturePtr feat, std::string& val)
{
    VmbErrorType err;
    VmbFeatureDataType type;

    err = feat->GetDataType(type);

    if (err != VmbErrorSuccess)
    {
        return err;
    }        

    switch (type)
    {
    case VmbFeatureDataBool:
        VmbBool_t boolVal;
        if (feat->GetValue(boolVal) == VmbErrorSuccess)
        {
            val = boolVal ? "true" : "false";
            return VmbErrorSuccess;
        }
        break;
    case VmbFeatureDataInt:
        VmbInt64_t intVal;
        if (feat->GetValue(intVal) == VmbErrorSuccess)
        {
            val = std::to_string(intVal);
            return VmbErrorSuccess;
        }
        break;
    case VmbFeatureDataFloat:
        double floatVal;
        if (feat->GetValue(floatVal) == VmbErrorSuccess)
        {
            val = std::to_string(floatVal);
            return VmbErrorSuccess;
        }
        break;
    case VmbFeatureDataEnum:
    case VmbFeatureDataString:
        std::string stringVal;
        if (feat->GetValue(stringVal) == VmbErrorSuccess)
        {
            val = stringVal;
            return VmbErrorSuccess;
        }
        break;
    }

    return VmbErrorNotSupported;
}

/**
 * \brief IFrameObserver implementation for asynchronous image acquisition and Chunk data access
 */
class FrameObserverChunk : public IFrameObserver
{
private:
    std::vector<std::string> m_chunkFeatures;

public:
    FrameObserverChunk(CameraPtr camera, std::vector<std::string> chunkFeatures) :
        IFrameObserver(camera),
        m_chunkFeatures(chunkFeatures)
    {}

    void FrameReceived(const FramePtr frame)
    {
        std::cout << "Frame Received" << std::endl;

        VmbFrameStatusType frameStatus;
        if (frame->GetReceiveStatus(frameStatus) == VmbErrorSuccess
            && frameStatus == VmbFrameStatusComplete)
        {
            // Access the Chunk data of the incoming frame. Chunk data accesible inside lambda function
            VmbErrorType err = frame->AccessChunkData([this](ChunkFeatureContainerPtr& chunkFeatures) -> VmbErrorType
                {
                    FeaturePtr feat;
                    VmbErrorType err;

                    // Try to access all Chunk features that were passed to the FrameObserverChunk instance
                    for (auto& chunkFeature : m_chunkFeatures)
                    {
                        std::cout << "\t";

                        // Get a specific Chunk feature via the FeatureContainer chunkFeatures
                        err = chunkFeatures->GetFeatureByName(chunkFeature.c_str(), feat);
                        if (err != VmbErrorSuccess)
                        {
                            std::cout << "Could not get Chunk feature \"" << chunkFeature << "\", err=" << std::to_string(err);
                            continue;
                        }

                        // The Chunk feature can be read like any other feature
                        std::string val;
                        err = GetFeatureValueAsString(feat, val);
                        if (err == VmbErrorSuccess)
                        {
                            std::cout << "" << chunkFeature << "=" << val;
                        }
                        else
                        {
                            std::cout << "Could not get value of Chunk feature \"" << chunkFeature << "\", err=" << std::to_string(err);
                        }

                        std::cout << std::endl;
                    }

                    // The return value of this function will passed to be the return value of Frame::AccessChunkData()
                    return VmbErrorSuccess;
                });
          
            if (err != VmbErrorSuccess)
            {
                std::cout << "Error accessing Chunk data: " << err << std::endl;
            }
        }
        else
        {
            std::cout << "Invalid frame" << std::endl;
        }

        std::cout << std::endl;
        m_pCamera->QueueFrame(frame);
    }
};

AcquisitionHelperChunk::AcquisitionHelperChunk() :
    AcquisitionHelperChunk(nullptr)
{
}

AcquisitionHelperChunk::AcquisitionHelperChunk(const char* cameraId) :
    m_vmbSystem(VmbSystem::GetInstance())
{
    VmbErrorType err = m_vmbSystem.Startup();

    if (err != VmbErrorSuccess)
    {
        throw std::runtime_error("Could not start API, err=" + std::to_string(err));
    }

    CameraPtrVector cameras;
    err = m_vmbSystem.GetCameras(cameras);
    if (err != VmbErrorSuccess)
    {
        m_vmbSystem.Shutdown();
        throw std::runtime_error("Could not get cameras, err=" + std::to_string(err));
    }

    if (cameras.empty())
    {
        m_vmbSystem.Shutdown();
        throw std::runtime_error("No cameras found.");
    }

    if (cameraId != nullptr)
    {
        err = m_vmbSystem.GetCameraByID(cameraId, m_camera);
        if (err != VmbErrorSuccess)
        {
            m_vmbSystem.Shutdown();
            throw std::runtime_error("No camera found with ID=" + std::string(cameraId) + ", err = " + std::to_string(err));
        }
    }
    else
    {
        m_camera = cameras[0];
    }

    err = m_camera->Open(VmbAccessModeFull);
    if (err != VmbErrorSuccess)
    {
        m_vmbSystem.Shutdown();
        throw std::runtime_error("Could not open camera, err=" + std::to_string(err));
    }

    std::string name;
    if (m_camera->GetName(name) == VmbErrorSuccess)
    {
        std::cout << "Opened Camera " << name << std::endl;
    }

    try
    {
        GigEAdjustPacketSize(m_camera);
    }
    catch (std::runtime_error& e)
    {
        m_vmbSystem.Shutdown();
        throw e;
    }
}

AcquisitionHelperChunk::~AcquisitionHelperChunk()
{
    try
    {
        Stop();
    }
    catch (std::runtime_error& e)
    {
        std::cout << e.what() << std::endl;
    }
    catch (...)
    {
        // ignore
    }

    m_vmbSystem.Shutdown();
}

void AcquisitionHelperChunk::EnableChunk()
{
    static std::vector<std::string> const defaultChunkFeatures
    {
        "ChunkFrameID",
        "ChunkTimestamp",
        "ChunkExposureTime",
        "ChunkGain"
    };

    FeaturePtr feat;

    if (!( (m_camera->GetFeatureByName("ChunkModeActive", feat) == VmbErrorSuccess)
           && (feat->SetValue(false) == VmbErrorSuccess) ))
    {
        std::cout << "Could not set ChunkModeActive. Does the camera provide Chunk Data?" << std::endl;
        return;
    }
    else {
        for (const auto& chunkFeature : defaultChunkFeatures)
        {
            static const std::string chunkPrefix = "Chunk";
            std::string chunkEnumEntry = chunkFeature.substr(chunkPrefix.size());

            if (m_camera->GetFeatureByName("ChunkSelector", feat) == VmbErrorSuccess)
            {
                if (feat->SetValue(chunkEnumEntry.c_str()) == VmbErrorSuccess)
                {
                    if (m_camera->GetFeatureByName("ChunkEnable", feat) == VmbErrorSuccess)
                    {
                        if (feat->SetValue(true) == VmbErrorSuccess)
                        {
                            m_chunkFeatures.push_back(chunkFeature);
                        }
                    }
                }
            }
        }
    }

    if (!m_chunkFeatures.empty())
    {
        std::cout << "Enabled the following Chunk features: " << std::endl;
        for (const auto& chunkFeature : m_chunkFeatures)
        {
            std::cout << "\t-" << chunkFeature << std::endl;
        }
        std::cout << std::endl;
    }
    else
    {
        std::cout << "Could not enable Chunk features. Does the camera provide Chunk Data?" << std::endl;
        return;
    }

    if (!( (m_camera->GetFeatureByName("ChunkModeActive", feat) == VmbErrorSuccess)
           && (feat->SetValue(true) == VmbErrorSuccess) ))
    {
        std::cout << "Could not set ChunkModeActive. Does the camera provide Chunk Data?" << std::endl;
        return;
    }
}

void AcquisitionHelperChunk::Start()
{
    VmbErrorType err = m_camera->StartContinuousImageAcquisition(5, IFrameObserverPtr(new FrameObserverChunk(m_camera, m_chunkFeatures)));
    if (err != VmbErrorSuccess)
    {
        throw std::runtime_error("Could not start acquisition, err=" + std::to_string(err));
    }
}

void AcquisitionHelperChunk::Stop()
{
    VmbErrorType err = m_camera->StopContinuousImageAcquisition();
    if (err != VmbErrorSuccess)
    {
        throw std::runtime_error("Could not stop acquisition, err=" + std::to_string(err));
    }
}

} // namespace Examples
} // namespace VmbCPP