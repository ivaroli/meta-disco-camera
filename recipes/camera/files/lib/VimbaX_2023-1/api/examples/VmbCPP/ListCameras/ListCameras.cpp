/*=============================================================================
  Copyright (C) 2012 - 2016 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        ListCameras.cpp

  Description: The ListCameras example will list all available cameras that
               are found by VmbCPP.

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

#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>

#include "ListCameras.h"

#include <VmbCPP/VmbCPP.h>


namespace VmbCPP {
    namespace Examples {

        /**printing camera info for a camera.
        *\note this function is used with for_each and is called for each camera in range cameras.begin(), cameras.end()
        */
        void PrintCameraInfo(const CameraPtr& camera)
        {
            std::string strID;
            std::string strName;
            std::string strModelName;
            std::string strSerialNumber;
            std::string strInterfaceID;
            TransportLayerPtr pTransportLayer;
            std::string strTransportLayerID;
            std::string strTransportLayerPath;

            std::ostringstream ErrorStream;

            VmbErrorType err = camera->GetID(strID);
            if (VmbErrorSuccess != err)
            {
                ErrorStream << "[Could not get camera ID. Error code: " << err << "]";
                strID = ErrorStream.str();
            }

            err = camera->GetName(strName);
            if (VmbErrorSuccess != err)
            {
                ErrorStream << "[Could not get camera name. Error code: " << err << "]";
                strName = ErrorStream.str();
            }

            err = camera->GetModel(strModelName);
            if (VmbErrorSuccess != err)
            {
                ErrorStream << "[Could not get camera mode name. Error code: " << err << "]";
                strModelName = ErrorStream.str();
            }

            err = camera->GetSerialNumber(strSerialNumber);
            if (VmbErrorSuccess != err)
            {
                ErrorStream << "[Could not get camera serial number. Error code: " << err << "]";
                strSerialNumber = ErrorStream.str();
            }

            err = camera->GetInterfaceID(strInterfaceID);
            if (VmbErrorSuccess != err)
            {
                ErrorStream << "[Could not get interface ID. Error code: " << err << "]";
                strInterfaceID = ErrorStream.str();
            }

            err = camera->GetTransportLayer(pTransportLayer);
            err = pTransportLayer->GetID(strTransportLayerID);
            if (VmbErrorSuccess != err)
            {
                ErrorStream << "[Could not get transport layer ID. Error code: " << err << "]";
                strTransportLayerID = ErrorStream.str();
            }
            err = pTransportLayer->GetPath(strTransportLayerPath);
            if (VmbErrorSuccess != err)
            {
                ErrorStream << "[Could not get transport layer path. Error code: " << err << "]";
                strTransportLayerPath = ErrorStream.str();
            }

            std::cout 
                << "/// Camera Name           : " << strName << "\n"
                << "/// Model Name            : " << strModelName << "\n"
                << "/// Camera ID             : " << strID << "\n"
                << "/// Serial Number         : " << strSerialNumber << "\n"
                << "/// @ Interface ID        : " << strInterfaceID << "\n"
                << "/// @ TransportLayer ID   : " << strTransportLayerID << "\n"
                << "/// @ TransportLayer Path : " << strTransportLayerPath << "\n\n";
        }

        //
        // Starts Vimba
        // Gets all connected cameras
        // And prints out information about the camera name, model name, serial number, ID and the corresponding interface ID
        //
        void ListCameras::Print()
        {
            VmbSystem& sys = VmbSystem::GetInstance();  // Get a reference to the VimbaSystem singleton

            VmbVersionInfo_t versionInfo;
            sys.QueryVersion(versionInfo);
            std::cout << "Vmb Version Major: " << versionInfo.major << " Minor: " << versionInfo.minor << " Patch: " << versionInfo.patch << "\n\n";

            VmbErrorType err = sys.Startup();           // Initialize the Vmb API
            if (VmbErrorSuccess == err)
            {

                TransportLayerPtrVector transportlayers;             // A vector of std::shared_ptr<AVT::VmbAPI::TransportLayer> objects
                err = sys.GetTransportLayers(transportlayers);       // Fetch all transport layers
                if (VmbErrorSuccess == err) std::cout << "TransportLayers found: " << transportlayers.size() << "\n";

                InterfacePtrVector interfaces;             // A vector of std::shared_ptr<AVT::VmbAPI::Interface> objects
                err = sys.GetInterfaces(interfaces);       // Fetch all interfaces
                if (VmbErrorSuccess == err) std::cout << "Interfaces found: " << interfaces.size() << "\n";

                CameraPtrVector cameras;                // A vector of std::shared_ptr<AVT::VmbAPI::Camera> objects
                err = sys.GetCameras(cameras);          // Fetch all cameras
                if (VmbErrorSuccess == err)
                {
                    std::cout << "Cameras found: " << cameras.size() << "\n\n";

                    // Query all static details of all known cameras and print them out.
                    // We don't have to open the cameras for that.
                    std::for_each(cameras.begin(), cameras.end(), PrintCameraInfo);
                }
                else
                {
                    std::cout << "Could not list cameras. Error code: " << err << "\n";
                }

                sys.Shutdown();                           // Close Vimba
            }
            else
            {
                std::cout << "Could not start system. Error code: " << err << "\n";
            }
        }

    }
}

