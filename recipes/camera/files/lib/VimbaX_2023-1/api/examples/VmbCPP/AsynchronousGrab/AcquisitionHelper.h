/*=============================================================================
  Copyright (C) 2012 - 2022 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        AcquisitionHelper.h

  Description: Helper class for acquiring images with VmbCPP

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

#ifndef ASYNCHRONOUS_GRAB_H_
#define ASYNCHRONOUS_GRAB_H_

#include <VmbCPP/VmbCPP.h>

namespace VmbCPP {
namespace Examples {

class AcquisitionHelper
{
private:
    VmbSystem&  m_vmbSystem;
    CameraPtr   m_camera;

public:
    /**
     * \brief The constructor will initialize the API and open the given camera
     *
     * \param[in] pCameraId  zero terminated C string with the camera id for the camera to be used
     */
    AcquisitionHelper(const char* cameraId);

    /**
     * \brief The constructor will initialize the API and open the first available camera
     */
    AcquisitionHelper();

    /**
     * \brief The destructor will stop the acquisition and shutdown the API
     */
    ~AcquisitionHelper();

    /**
     * \brief Start the acquisition.
     */
    void Start();

    /**
     * \brief Stop the acquisition.
     */
    void Stop();
};

}} // namespace VmbCPP

#endif
